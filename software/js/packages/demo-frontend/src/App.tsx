import React, {useEffect, useMemo, useRef, useState} from 'react'
import './App.css'
import io from 'socket.io-client'
import Typography from '@mui/material/Typography'
import Container from '@mui/material/Container'
import ToggleButton from '@mui/material/ToggleButton'
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup'
import {PB} from 'smartknobjs-proto'
import {VideoInfo} from './types'
import {Card, CardContent} from '@mui/material'
import {exhaustiveCheck, findNClosest, INT32_MIN, lerp, NoUndefinedField} from './util'
import {groupBy, parseInt} from 'lodash'

const socket = io()

const MIN_ZOOM = 0.01
const MAX_ZOOM = 60

const PIXELS_PER_POSITION = 10

enum Mode {
    Scroll = 'Scroll',
    Frames = 'Frames',
    Speed = 'Speed',
}

type State = {
    mode: Mode
    playbackSpeed: number
    currentFrame: number
    zoomTimelinePixelsPerFrame: number
}

export type AppProps = {
    info: VideoInfo
}
export const App: React.FC<AppProps> = ({info}) => {
    const [isConnected, setIsConnected] = useState(socket.connected)
    const [state, setState] = useState<NoUndefinedField<PB.ISmartKnobState>>(
        PB.SmartKnobState.toObject(PB.SmartKnobState.create({config: PB.SmartKnobConfig.create()}), {
            defaults: true,
        }) as NoUndefinedField<PB.ISmartKnobState>,
    )
    const [derivedState, setDerivedState] = useState<State>({
        mode: Mode.Scroll,
        playbackSpeed: 0,
        currentFrame: 0,
        zoomTimelinePixelsPerFrame: 0.1,
    })

    useMemo(() => {
        setDerivedState((cur) => {
            const modeText = state.config.text
            if (modeText === Mode.Scroll) {
                const rawFrame = Math.trunc(
                    ((state.currentPosition + state.subPositionUnit) * PIXELS_PER_POSITION) /
                        cur.zoomTimelinePixelsPerFrame,
                )
                return {
                    mode: Mode.Scroll,
                    playbackSpeed: 0,
                    currentFrame: Math.min(Math.max(rawFrame, 0), info.totalFrames - 1),
                    zoomTimelinePixelsPerFrame: cur.zoomTimelinePixelsPerFrame,
                }
            } else if (modeText === Mode.Frames) {
                return {
                    mode: Mode.Frames,
                    playbackSpeed: 0,
                    currentFrame: state.currentPosition ?? 0,
                    zoomTimelinePixelsPerFrame: cur.zoomTimelinePixelsPerFrame,
                }
            } else if (modeText === Mode.Speed) {
                const normalizedWholeValue = state.currentPosition
                const normalizedFractional =
                    Math.sign(state.subPositionUnit) *
                    lerp(state.subPositionUnit * Math.sign(state.subPositionUnit), 0.1, 0.9, 0, 1)
                const normalized = normalizedWholeValue + normalizedFractional
                const speed = Math.sign(normalized) * Math.pow(2, Math.abs(normalized) - 1)
                return {
                    mode: Mode.Speed,
                    playbackSpeed: speed,
                    currentFrame: cur.currentFrame,
                    zoomTimelinePixelsPerFrame: cur.zoomTimelinePixelsPerFrame,
                }
            }
            return cur
        })
    }, [state.config.text, state.currentPosition, state.subPositionUnit])

    const totalPositions = Math.ceil((info.totalFrames * derivedState.zoomTimelinePixelsPerFrame) / PIXELS_PER_POSITION)
    const detentPositions = useMemo(() => {
        // Always include the first and last positions at detents
        const positionsToFrames = groupBy([0, ...info.boundaryFrames, info.totalFrames - 1], (frame) =>
            Math.round((frame * derivedState.zoomTimelinePixelsPerFrame) / PIXELS_PER_POSITION),
        )
        console.log(JSON.stringify(positionsToFrames))
        return positionsToFrames
    }, [info.boundaryFrames, totalPositions, derivedState.zoomTimelinePixelsPerFrame])

    // Continuous config updates for scrolling, to update detent positions
    useMemo(() => {
        if (derivedState.mode === Mode.Scroll) {
            const config = PB.SmartKnobConfig.create({
                position: INT32_MIN,
                minPosition: 0,
                maxPosition: totalPositions - 1,
                positionWidthRadians: (8 * Math.PI) / 180,
                detentStrengthUnit: 2.5,
                endstopStrengthUnit: 1,
                snapPoint: 0.7,
                text: Mode.Scroll,
                detentPositions: findNClosest(Object.keys(detentPositions).map(parseInt), state.currentPosition, 5),
                snapPointBias: 0,
            })
            socket.emit('set_config', config)
        }
    }, [derivedState.mode, derivedState.zoomTimelinePixelsPerFrame, detentPositions, state.currentPosition])

    // For one-off config pushes, e.g. mode changes
    const pushConfig = (state: State) => {
        let config: PB.SmartKnobConfig
        if (state.mode === Mode.Scroll) {
            const position = Math.trunc((state.currentFrame * state.zoomTimelinePixelsPerFrame) / PIXELS_PER_POSITION)
            config = PB.SmartKnobConfig.create({
                position,
                minPosition: 0,
                maxPosition: Math.trunc(
                    ((info.totalFrames - 1) * state.zoomTimelinePixelsPerFrame) / PIXELS_PER_POSITION,
                ),
                positionWidthRadians: (8 * Math.PI) / 180,
                detentStrengthUnit: 2.5,
                endstopStrengthUnit: 1,
                snapPoint: 0.7,
                text: Mode.Scroll,
                detentPositions: findNClosest(Object.keys(detentPositions).map(parseInt), position, 5),
                snapPointBias: 0,
            })
        } else if (state.mode === Mode.Frames) {
            config = PB.SmartKnobConfig.create({
                position: state.currentFrame,
                minPosition: 0,
                maxPosition: info.totalFrames - 1,
                positionWidthRadians: (1.5 * Math.PI) / 180,
                detentStrengthUnit: 1,
                endstopStrengthUnit: 1,
                snapPoint: 1.1,
                text: Mode.Frames,
                detentPositions: [],
                snapPointBias: 0,
            })
        } else if (state.mode === Mode.Speed) {
            config = PB.SmartKnobConfig.create({
                position: state.playbackSpeed === 0 ? 0 : INT32_MIN,
                minPosition: state.currentFrame === 0 ? 0 : -6,
                maxPosition: state.currentFrame === info.totalFrames - 1 ? 0 : 6,
                positionWidthRadians: (60 * Math.PI) / 180,
                detentStrengthUnit: 1,
                endstopStrengthUnit: 1,
                snapPoint: 0.55,
                text: Mode.Speed,
                detentPositions: [],
                snapPointBias: 0.4,
            })
        } else {
            throw exhaustiveCheck(state.mode)
        }
        socket.emit('set_config', config)
    }

    const setCurrentFrame = (fn: (oldFrame: number) => number) => {
        setDerivedState((cur) => {
            const newState = {...cur}
            if (cur.mode === Mode.Speed) {
                newState.currentFrame = fn(cur.currentFrame)
            }
            return newState
        })
    }

    // Timer for speed-based playback
    useEffect(() => {
        const refreshInterval = 20
        const fps = info.frameRate * derivedState.playbackSpeed
        if (derivedState.mode === Mode.Speed && fps !== 0) {
            const timer = setInterval(() => {
                setCurrentFrame((oldFrame) => {
                    const newFrame = oldFrame + (fps * refreshInterval) / 1000

                    const oldFrameTrunc = Math.trunc(oldFrame)
                    const newFrameTrunc = Math.trunc(newFrame)

                    if (newFrame < 0 || newFrame >= info.totalFrames) {
                        const clampedNewFrame = Math.min(Math.max(newFrame, 0), info.totalFrames - 1)
                        if (oldFrame !== clampedNewFrame) {
                            // If we've hit a boundary, push a config to set the bounds
                            pushConfig({
                                mode: Mode.Speed,
                                playbackSpeed: 0,
                                currentFrame: Math.trunc(clampedNewFrame),
                                zoomTimelinePixelsPerFrame: derivedState.zoomTimelinePixelsPerFrame,
                            })
                        }
                        return clampedNewFrame
                    } else {
                        if (
                            (oldFrameTrunc === 0 && newFrameTrunc > 0) ||
                            (oldFrameTrunc === info.totalFrames - 1 && newFrameTrunc < info.totalFrames - 1)
                        ) {
                            // If we've left a boundary condition, push a config to reset the bounds
                            pushConfig({
                                mode: derivedState.mode,
                                playbackSpeed: 0,
                                currentFrame: newFrameTrunc,
                                zoomTimelinePixelsPerFrame: derivedState.zoomTimelinePixelsPerFrame,
                            })
                        }
                        return newFrame
                    }
                })
            }, refreshInterval)
            return () => clearInterval(timer)
        }
    }, [derivedState.mode, derivedState.playbackSpeed, info.totalFrames, info.frameRate])

    // Socket.io subscription
    useEffect(() => {
        socket.on('connect', () => {
            setIsConnected(true)
            pushConfig(derivedState)
        })

        socket.on('disconnect', () => {
            setIsConnected(false)
        })

        socket.on('state', (input: {pb: PB.SmartKnobState}) => {
            const {pb: state} = input
            const stateObj = PB.SmartKnobState.toObject(state, {
                defaults: true,
            }) as NoUndefinedField<PB.ISmartKnobState>
            setState(stateObj)
        })
        return () => {
            socket.off('connect')
            socket.off('disconnect')
            socket.off('state')
        }
    }, [])
    return (
        <>
            <Container component="main" maxWidth="md">
                <Card>
                    <CardContent>
                        <Typography component="h1" variant="h5">
                            Video Playback Control Demo
                        </Typography>
                        {isConnected || (
                            <Typography component="h6" variant="h6">
                                [Not connected]
                            </Typography>
                        )}
                        <ToggleButtonGroup
                            color="primary"
                            value={derivedState.mode}
                            exclusive
                            onChange={(e, value: Mode | null) => {
                                if (value === null) {
                                    return
                                }
                                pushConfig({
                                    ...derivedState,
                                    mode: value,
                                })
                            }}
                            aria-label="Mode"
                        >
                            {Object.keys(Mode).map((mode) => (
                                <ToggleButton value={mode} key={mode}>
                                    {mode}
                                </ToggleButton>
                            ))}
                        </ToggleButtonGroup>
                        <Typography>
                            Frame {Math.trunc(derivedState.currentFrame)} / {info.totalFrames - 1}
                            <br />
                            Speed {Math.trunc(derivedState.playbackSpeed * 10) / 10}
                        </Typography>
                    </CardContent>
                </Card>
                <Timeline
                    info={info}
                    currentFrame={derivedState.currentFrame}
                    zoomTimelinePixelsPerFrame={derivedState.zoomTimelinePixelsPerFrame}
                    adjustZoom={(factor) => {
                        setDerivedState((cur) => {
                            const newZoom = Math.min(
                                Math.max(cur.zoomTimelinePixelsPerFrame * factor, MIN_ZOOM),
                                MAX_ZOOM,
                            )
                            console.log(factor, newZoom)
                            return {
                                ...cur,
                                zoomTimelinePixelsPerFrame: newZoom,
                            }
                        })
                    }}
                />
                <Card>
                    <CardContent>
                        <div>{JSON.stringify(detentPositions)}</div>
                    </CardContent>
                </Card>
            </Container>
        </>
    )
}

export type TimelineProps = {
    info: VideoInfo
    currentFrame: number
    zoomTimelinePixelsPerFrame: number
    adjustZoom: (factor: number) => void
}
export const Timeline: React.FC<TimelineProps> = ({info, currentFrame, zoomTimelinePixelsPerFrame, adjustZoom}) => {
    const gradients = [
        'linear-gradient( 135deg, #FDEB71 10%, #F8D800 100%)',
        'linear-gradient( 135deg, #ABDCFF 10%, #0396FF 100%)',
        'linear-gradient( 135deg, #FEB692 10%, #EA5455 100%)',
        'linear-gradient( 135deg, #CE9FFC 10%, #7367F0 100%)',
        'linear-gradient( 135deg, #90F7EC 10%, #32CCBC 100%)',
    ]

    const timelineRef = useRef<HTMLDivElement>(null)
    const cursorRef = useRef<HTMLDivElement>(null)

    useEffect(() => {
        const handleWheel = (event: HTMLElementEventMap['wheel']) => {
            const delta = event.deltaY
            if (delta) {
                event.preventDefault()
                adjustZoom(1 - delta / 500)
            }
        }

        timelineRef.current?.addEventListener('wheel', handleWheel)
        return () => {
            timelineRef.current?.removeEventListener('wheel', handleWheel)
        }
    }, [])

    useEffect(() => {
        cursorRef.current?.scrollIntoView()
    }, [currentFrame, zoomTimelinePixelsPerFrame])
    return (
        <div
            className="timeline-container"
            ref={timelineRef}
            style={{
                width: '100%',
                margin: '10px auto',
                overflowX: 'scroll',
            }}
        >
            <div
                className="timeline"
                style={{
                    position: 'relative',
                    display: 'inline-block',
                    height: '80px',
                    width: `${zoomTimelinePixelsPerFrame * info.totalFrames}px`,
                    backgroundColor: '#dde',
                }}
            >
                {[...info.boundaryFrames, info.totalFrames].map((f, i, a) => {
                    const lengthFrames = f - (a[i - 1] ?? 0)
                    const widthPixels = zoomTimelinePixelsPerFrame * lengthFrames
                    return (
                        <div
                            key={`clip-${f}`}
                            className="video-clip"
                            style={{
                                position: 'relative',
                                display: 'inline-block',
                                top: '10px',
                                height: '60px',
                                width: `${widthPixels}px`,
                                backgroundImage: gradients[i % gradients.length],
                            }}
                        ></div>
                    )
                })}
                <div
                    className="playback-cursor"
                    ref={cursorRef}
                    style={{
                        position: 'absolute',
                        display: 'inline-block',
                        left: `${zoomTimelinePixelsPerFrame * Math.trunc(currentFrame)}px`,
                        width: `${Math.max(zoomTimelinePixelsPerFrame, 1)}px`,
                        height: '100%',
                        backgroundColor: 'rgba(255, 0, 0, 0.4)',
                        borderLeft: '1px solid red',
                    }}
                ></div>
            </div>
        </div>
    )
}

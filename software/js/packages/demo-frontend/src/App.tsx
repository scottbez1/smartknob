import React, {useEffect, useState} from 'react'
import './App.css'
import io from 'socket.io-client'
import Typography from '@mui/material/Typography'
import Container from '@mui/material/Container'
import ToggleButton from '@mui/material/ToggleButton'
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup'
import {PB} from 'smartknobjs-proto'
import {VideoInfo} from './types'
import {Card, CardActions, CardContent} from '@mui/material'
import {exhaustiveCheck, INT32_MIN, lerp, NoUndefinedField} from './util'

const socket = io()

// const MIN_ZOOM = 0.01
// const MAX_ZOOM = 40

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
    const [state, setState] = useState<string>('')

    const [derivedState, setDerivedState] = useState<State>({
        mode: Mode.Scroll,
        playbackSpeed: 0,
        currentFrame: 0,
        zoomTimelinePixelsPerFrame: 0.1,
    })

    const pushConfig = (state: State) => {
        let config: PB.SmartKnobConfig
        if (state.mode === Mode.Scroll) {
            // TODO
            config = PB.SmartKnobConfig.create({
                position: 0,
                minPosition: 0,
                maxPosition: 31,
                positionWidthRadians: (7 * Math.PI) / 180,
                detentStrengthUnit: 2.5,
                endstopStrengthUnit: 1,
                snapPoint: 0.7,
                text: Mode.Scroll,
                detentPositions: [0, 10, 21, 22],
                snapPointBias: 0,
            })
        } else if (state.mode === Mode.Frames) {
            config = PB.SmartKnobConfig.create({
                position: state.currentFrame,
                minPosition: 0,
                maxPosition: info.totalFrames - 1,
                positionWidthRadians: (1 * Math.PI) / 180,
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
                        // TODO: figure out how to avoid sending this config repeatedly
                        pushConfig({
                            mode: Mode.Speed,
                            playbackSpeed: 0,
                            currentFrame: Math.trunc(clampedNewFrame),
                            zoomTimelinePixelsPerFrame: derivedState.zoomTimelinePixelsPerFrame,
                        })
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
            setState(JSON.stringify(stateObj, undefined, 4))
            setDerivedState((cur) => {
                const modeText = stateObj.config.text
                if (modeText === Mode.Scroll) {
                    return {
                        mode: Mode.Scroll,
                        playbackSpeed: 0,
                        currentFrame: cur.currentFrame, // TODO - map from state and zoom
                        zoomTimelinePixelsPerFrame: cur.zoomTimelinePixelsPerFrame,
                    }
                } else if (modeText === Mode.Frames) {
                    return {
                        mode: Mode.Frames,
                        playbackSpeed: 0,
                        currentFrame: stateObj.currentPosition ?? 0,
                        zoomTimelinePixelsPerFrame: cur.zoomTimelinePixelsPerFrame,
                    }
                } else if (modeText === Mode.Speed) {
                    const normalizedWholeValue = stateObj.currentPosition
                    const normalizedFractional =
                        Math.sign(stateObj.subPositionUnit) *
                        lerp(stateObj.subPositionUnit * Math.sign(stateObj.subPositionUnit), 0.1, 0.9, 0, 1)
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
        })
        return () => {
            socket.off('connect')
            socket.off('disconnect')
            socket.off('state')
        }
    }, [])
    return (
        <Container component="main" maxWidth="sm">
            <Card>
                <CardContent>
                    <Typography component="h1" variant="h5">
                        Video Playback Control Demo
                    </Typography>
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
                    <Typography component="h1" variant="h5">
                        Frame {Math.trunc(derivedState.currentFrame)} / {info.totalFrames - 1}
                        <br />
                        Speed {Math.trunc(derivedState.playbackSpeed * 10) / 10}
                    </Typography>
                </CardContent>
                <CardActions>
                    {/* {derivedState.mode === Mode.Speed && (
                        <Button
                            size="small"
                            onClick={() => {
                                if (derivedState.playbackSpeed !== 0) {
                                    setPlaybackSpeed(0)
                                } else {
                                    setPlaybackSpeed(1)
                                }
                                pushCurrentConfig()
                            }}
                        >
                            Play
                        </Button>
                    )} */}
                </CardActions>
            </Card>
            <Card>
                <CardContent>
                    <div>{isConnected ? <code>{state}</code> : <p>Not connected</p>}</div>
                </CardContent>
            </Card>
        </Container>
    )
}

export default App

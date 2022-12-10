import React, {useEffect, useState} from 'react'
import './App.css'
import io from 'socket.io-client'
import Typography from '@mui/material/Typography'
import Container from '@mui/material/Container'
import ToggleButton from '@mui/material/ToggleButton'
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup'
import {PB} from 'smartknobjs-proto'
import {VideoInfo} from './types'
import {Button, Card, CardActions, CardContent} from '@mui/material'
import {exhaustiveCheck} from './util'

const socket = io()

enum Mode {
    Scroll = 'Scroll',
    Frames = 'Frames',
    Speed = 'Speed',
}

export type AppProps = {
    info: VideoInfo
}
export const App: React.FC<AppProps> = ({info}) => {
    const [isConnected, setIsConnected] = useState(socket.connected)
    const [state, setState] = useState<string>('')

    const [mode, setRawMode] = useState<Mode>(Mode.Scroll)

    const [playbackSpeed, setPlaybackSpeed] = useState<number>(0)
    const [currentFrame, setCurrentFrame] = useState<number>(0)

    const pushCurrentConfig = () => {
        let config: PB.SmartKnobConfig
        if (mode === Mode.Scroll) {
            // TODO
            config = PB.SmartKnobConfig.create({
                numPositions: 32,
                position: 0,
                positionWidthRadians: (7 * Math.PI) / 180,
                detentStrengthUnit: 2.5,
                endstopStrengthUnit: 1,
                snapPoint: 0.7,
                text: Mode.Scroll,
                detentPositions: [0, 10, 21, 22],
                snapPointBias: 0,
                snapPointBiasCenterPosition: 0,
            })
        } else if (mode === Mode.Frames) {
            config = PB.SmartKnobConfig.create({
                numPositions: info.totalFrames,
                position: currentFrame,
                positionWidthRadians: (1 * Math.PI) / 180,
                detentStrengthUnit: 1,
                endstopStrengthUnit: 1,
                snapPoint: 1.1,
                text: Mode.Frames,
                detentPositions: [],
                snapPointBias: 0,
                snapPointBiasCenterPosition: 0,
            })
        } else if (mode === Mode.Speed) {
            config = PB.SmartKnobConfig.create({
                numPositions: 13,
                position: 6 + Math.trunc(playbackSpeed),
                positionWidthRadians: (60 * Math.PI) / 180,
                detentStrengthUnit: 1,
                endstopStrengthUnit: 1,
                snapPoint: 0.55,
                text: Mode.Speed,
                detentPositions: [],
                snapPointBias: 0.4,
                snapPointBiasCenterPosition: 6,
            })
        } else {
            throw exhaustiveCheck(mode)
        }
        socket.emit('set_config', config)
    }

    useEffect(() => {
        const fps = info.frameRate * playbackSpeed
        const msPerFrame = 1000 / fps
        if (fps !== 0) {
            const timer = setInterval(() => {
                setCurrentFrame((oldFrame) => {
                    const newFrame = oldFrame + Math.sign(playbackSpeed)
                    if (newFrame < 0 || newFrame >= info.totalFrames) {
                        setPlaybackSpeed(0)
                        pushCurrentConfig()
                        return oldFrame
                    } else {
                        return newFrame
                    }
                })
            }, msPerFrame)
            return () => clearInterval(timer)
        }
    }, [playbackSpeed, info.totalFrames, info.frameRate])

    const setMode = (m: Mode) => {
        setRawMode(m)
        if (m !== Mode.Speed) {
            setPlaybackSpeed(0)
        }
        pushCurrentConfig()
    }

    useEffect(() => {
        socket.on('connect', () => {
            setIsConnected(true)
        })

        socket.on('disconnect', () => {
            setIsConnected(false)
        })

        socket.on('state', (input: {pb: PB.SmartKnobState}) => {
            const {pb: state} = input
            const stateObj = PB.SmartKnobState.toObject(state, {defaults: true})
            setState(JSON.stringify(stateObj, undefined, 4))
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
                        value={mode}
                        exclusive
                        onChange={(e, value: Mode | null) => {
                            if (value === null) {
                                return
                            }
                            setMode(value)
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
                        Frame {currentFrame} / {info.totalFrames - 1}
                    </Typography>
                </CardContent>
                <CardActions>
                    {mode === Mode.Speed && (
                        <Button
                            size="small"
                            onClick={() => {
                                if (playbackSpeed !== 0) {
                                    setPlaybackSpeed(0)
                                } else {
                                    setPlaybackSpeed(1)
                                }
                                pushCurrentConfig()
                            }}
                        >
                            Play
                        </Button>
                    )}
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

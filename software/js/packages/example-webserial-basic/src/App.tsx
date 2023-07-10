import React, {useEffect, useState} from 'react'
import Typography from '@mui/material/Typography'
import Container from '@mui/material/Container'
import {PB} from 'smartknobjs-proto'
import {Box, Button, CardActions, Paper, TextField} from '@mui/material'
import {NoUndefinedField} from './util'
import {SmartKnobWebSerial} from 'smartknobjs-webserial'

type Config = NoUndefinedField<PB.ISmartKnobConfig>

const defaultConfig: Config = {
    position: 0,
    subPositionUnit: 0,
    positionNonce: Math.floor(Math.random() * 255),
    minPosition: 0,
    maxPosition: 20,
    positionWidthRadians: (15 * Math.PI) / 180,
    detentStrengthUnit: 0.5,
    endstopStrengthUnit: 1,
    snapPoint: 0.7,
    text: 'Hello from\nweb serial!',
    detentPositions: [],
    snapPointBias: 0,
    ledHue: 0,
}

export type AppProps = object
export const App: React.FC<AppProps> = () => {
    const [smartKnob, setSmartKnob] = useState<SmartKnobWebSerial | null>(null)
    const [smartKnobState, setSmartKnobState] = useState<NoUndefinedField<PB.ISmartKnobState>>(
        PB.SmartKnobState.toObject(PB.SmartKnobState.create({config: PB.SmartKnobConfig.create()}), {
            defaults: true,
        }) as NoUndefinedField<PB.ISmartKnobState>,
    )
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    const [smartKnobConfig, setSmartKnobConfig] = useState<Config>(defaultConfig)
    useEffect(() => {
        console.log('send config', smartKnobConfig)
        smartKnob?.sendConfig(PB.SmartKnobConfig.create(smartKnobConfig))
    }, [
        smartKnob,
        smartKnobConfig.position,
        smartKnobConfig.subPositionUnit,
        smartKnobConfig.positionNonce,
        smartKnobConfig.minPosition,
        smartKnobConfig.maxPosition,
        smartKnobConfig.positionWidthRadians,
        smartKnobConfig.detentStrengthUnit,
        smartKnobConfig.endstopStrengthUnit,
        smartKnobConfig.snapPoint,
        smartKnobConfig.text,
        smartKnobConfig.detentPositions,
        smartKnobConfig.snapPointBias,
    ])
    const [pendingSmartKnobConfig, setPendingSmartKnobConfig] = useState<{[P in keyof Config]: string}>(() => {
        return Object.fromEntries(Object.entries(defaultConfig).map(([key, value]) => [key, String(value)])) as {
            [P in keyof Config]: string
        }
    })

    const connectToSerial = async () => {
        try {
            if (navigator.serial) {
                const serialPort = await navigator.serial.requestPort({
                    filters: SmartKnobWebSerial.USB_DEVICE_FILTERS,
                })
                serialPort.addEventListener('disconnect', () => {
                    setSmartKnob(null)
                })
                const smartKnob = new SmartKnobWebSerial(serialPort, (message) => {
                    if (message.payload === 'smartknobState' && message.smartknobState !== null) {
                        const state = PB.SmartKnobState.create(message.smartknobState)
                        const stateObj = PB.SmartKnobState.toObject(state, {
                            defaults: true,
                        }) as NoUndefinedField<PB.ISmartKnobState>
                        setSmartKnobState(stateObj)
                    } else if (message.payload === 'log' && message.log !== null) {
                        console.log('LOG from smartknob', message.log?.msg)
                    }
                })
                setSmartKnob(smartKnob)
                const loop = smartKnob.openAndLoop()
                console.log('FIXME')
                smartKnob.sendConfig(PB.SmartKnobConfig.create(smartKnobConfig))
                await loop
            } else {
                console.error('Web Serial API is not supported in this browser.')
                setSmartKnob(null)
            }
        } catch (error) {
            console.error('Error with serial port:', error)
            setSmartKnob(null)
        }
    }

    return (
        <>
            <Container component="main" maxWidth="lg">
                <Paper variant="outlined" sx={{my: {xs: 3, md: 6}, p: {xs: 2, md: 3}}}>
                    <Typography component="h1" variant="h5">
                        Basic SmartKnob Web Serial Demo
                    </Typography>
                    {smartKnob !== null ? (
                        <>
                            <Box
                                component="form"
                                sx={{
                                    '& .MuiTextField-root': {m: 1, width: '25ch'},
                                }}
                                noValidate
                                autoComplete="off"
                                onSubmit={(event) => {
                                    event.preventDefault()
                                    setSmartKnobConfig({
                                        position: parseInt(pendingSmartKnobConfig.position) || 0,
                                        subPositionUnit: parseFloat(pendingSmartKnobConfig.subPositionUnit) || 0,
                                        positionNonce: parseInt(pendingSmartKnobConfig.positionNonce) || 0,
                                        minPosition: parseInt(pendingSmartKnobConfig.minPosition) || 0,
                                        maxPosition: parseInt(pendingSmartKnobConfig.maxPosition) || 0,
                                        positionWidthRadians:
                                            parseFloat(pendingSmartKnobConfig.positionWidthRadians) || 0,
                                        detentStrengthUnit: parseFloat(pendingSmartKnobConfig.detentStrengthUnit) || 0,
                                        endstopStrengthUnit:
                                            parseFloat(pendingSmartKnobConfig.endstopStrengthUnit) || 0,
                                        snapPoint: parseFloat(pendingSmartKnobConfig.snapPoint) || 0,
                                        text: pendingSmartKnobConfig.text,
                                        detentPositions: [],
                                        snapPointBias: parseFloat(pendingSmartKnobConfig.snapPointBias) || 0,
                                        ledHue: 0,
                                    })
                                }}
                            >
                                <TextField
                                    label="Position"
                                    value={pendingSmartKnobConfig.position}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                position: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <TextField
                                    label="Sub-position unit"
                                    value={pendingSmartKnobConfig.subPositionUnit}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                subPositionUnit: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <TextField
                                    label="Position nonce"
                                    value={pendingSmartKnobConfig.positionNonce}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                positionNonce: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <TextField
                                    label="Min position"
                                    value={pendingSmartKnobConfig.minPosition}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                minPosition: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <TextField
                                    label="Max position"
                                    value={pendingSmartKnobConfig.maxPosition}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                maxPosition: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <TextField
                                    label="Position width (radians)"
                                    value={pendingSmartKnobConfig.positionWidthRadians}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                positionWidthRadians: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <TextField
                                    label="Detent strength unit"
                                    value={pendingSmartKnobConfig.detentStrengthUnit}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                detentStrengthUnit: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <TextField
                                    label="Endstop strength unit"
                                    value={pendingSmartKnobConfig.endstopStrengthUnit}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                endstopStrengthUnit: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <TextField
                                    label="Snap point"
                                    value={pendingSmartKnobConfig.snapPoint}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                snapPoint: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <TextField
                                    label="Snap point bias"
                                    value={pendingSmartKnobConfig.snapPointBias}
                                    type="number"
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                snapPointBias: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <TextField
                                    label="Text"
                                    value={pendingSmartKnobConfig.text}
                                    multiline
                                    onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                                        setPendingSmartKnobConfig((cur) => {
                                            return {
                                                ...cur,
                                                text: event.target.value,
                                            }
                                        })
                                    }}
                                />
                                <br />
                                <Button type="submit" variant="contained">
                                    Apply
                                </Button>
                            </Box>
                            <pre>{JSON.stringify(smartKnobState, undefined, 2)}</pre>
                        </>
                    ) : navigator.serial ? (
                        <CardActions>
                            <Button onClick={connectToSerial} variant="contained">
                                Connect via Web Serial
                            </Button>
                        </CardActions>
                    ) : (
                        <Typography>
                            Sorry, Web Serial API isn't available in your browser. Try the latest version of Chrome.
                        </Typography>
                    )}
                </Paper>
            </Container>
        </>
    )
}

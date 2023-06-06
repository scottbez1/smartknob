import SerialPort = require('serialport')
import {SmartKnob} from 'smartknobjs'
import {PB} from 'smartknobjs-proto'

import {Server, Socket} from 'socket.io'

const io = new Server(parseInt(process.env.PORT ?? '3001'))

const start = async () => {
    const ports = await SerialPort.list()

    const matchingPorts = ports.filter((portInfo) => {
        // Implement a check for your device's vendor+product+serial
        // (this is more robust than the alternative of just hardcoding a "path" like "/dev/ttyUSB0")
        return (
            (portInfo.vendorId?.toLowerCase() === '1a86'.toLowerCase() &&
                portInfo.productId?.toLowerCase() === '7523'.toLowerCase()) ||
            (portInfo.vendorId?.toLowerCase() === '303a'.toLowerCase() &&
                portInfo.productId?.toLowerCase() === '1001'.toLowerCase())
            // && portInfo.serialNumber === 'DEADBEEF'
        )
    })

    if (matchingPorts.length < 1) {
        console.error(`No smartknob usb serial port found! ${JSON.stringify(ports, undefined, 4)}`)
        return
    } else if (matchingPorts.length > 1) {
        console.error(`Multiple smartknob usb serial ports found: ${JSON.stringify(matchingPorts, undefined, 4)}`)
        return
    }

    const portInfo = matchingPorts[0]
    console.info('Connecting to ', portInfo)

    let lastLoggedState: PB.ISmartKnobState | undefined
    const smartknob = new SmartKnob(portInfo.path, (message: PB.FromSmartKnob) => {
        if (message.payload === 'log' && message.log) {
            console.log('LOG', message.log.msg)
        } else if (message.payload === 'smartknobState' && message.smartknobState) {
            const state = PB.SmartKnobState.toObject(message.smartknobState as PB.SmartKnobState, {defaults: true})
            io.emit('state', {pb: message.smartknobState})
            if (
                message.smartknobState.currentPosition !== lastLoggedState?.currentPosition ||
                Math.abs((message.smartknobState.subPositionUnit ?? 0) - (lastLoggedState?.subPositionUnit ?? 0)) > 1
            ) {
                console.log(`State:\n${JSON.stringify(state, undefined, 4)}`)
                lastLoggedState = message.smartknobState
            }
        }
    })
    smartknob.sendConfig(
        PB.SmartKnobConfig.create({
            detentStrengthUnit: 1,
            endstopStrengthUnit: 1,
            position: 0,
            minPosition: -5,
            maxPosition: 5,
            positionWidthRadians: (10 * Math.PI) / 180,
            snapPoint: 1.1,
            text: 'From TS!',
        }),
    )

    let currentSocket: Socket | null = null
    io.on('connection', (socket) => {
        if (currentSocket !== null) {
            currentSocket.disconnect(true)
        }
        currentSocket = socket
        socket.on('set_config', (config) => {
            console.log(config)
            smartknob.sendConfig(config)
        })
    })
}

start()

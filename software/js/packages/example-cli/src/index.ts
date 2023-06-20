import SerialPort = require('serialport')
import {SmartKnobNode} from 'smartknobjs-node'
import {PB} from 'smartknobjs-proto'

const main = async () => {
    const ports = await SerialPort.list()

    const matchingPorts = ports.filter((portInfo) => {
        return SmartKnobNode.USB_DEVICE_FILTERS.some(
            (f) =>
                f.usbVendorId.toString(16) === portInfo.vendorId && f.usbProductId.toString(16) === portInfo.productId,
        )
    })

    if (matchingPorts.length < 1) {
        console.error(`No smartknob usb serial port found! ${JSON.stringify(ports, undefined, 4)}`)
        return
    } else if (matchingPorts.length > 1) {
        console.error(
            `Multiple possible smartknob usb serial ports found: ${JSON.stringify(matchingPorts, undefined, 4)}`,
        )
        return
    }

    const portInfo = matchingPorts[0]

    let lastLoggedState: PB.ISmartKnobState | undefined
    const smartknob = new SmartKnobNode(portInfo.path, (message: PB.FromSmartKnob) => {
        if (message.payload === 'log' && message.log) {
            console.log('LOG', message.log.msg)
        } else if (message.payload === 'smartknobState' && message.smartknobState) {
            // Only log if it's a significant change (major position change, or at least 5 degrees)
            const radianChange =
                (message.smartknobState.subPositionUnit ?? 0) *
                    (message.smartknobState.config?.positionWidthRadians ?? 0) -
                (lastLoggedState?.subPositionUnit ?? 0) * (lastLoggedState?.config?.positionWidthRadians ?? 0)
            if (
                message.smartknobState.currentPosition !== lastLoggedState?.currentPosition ||
                (Math.abs(radianChange) * 180) / Math.PI > 5
            ) {
                console.log(
                    `State:\n${JSON.stringify(
                        PB.SmartKnobState.toObject(message.smartknobState as PB.SmartKnobState, {defaults: true}),
                        undefined,
                        4,
                    )}`,
                )
                lastLoggedState = message.smartknobState
            }
        }
    })
    smartknob.sendConfig(
        PB.SmartKnobConfig.create({
            detentStrengthUnit: 1,
            endstopStrengthUnit: 1,
            position: 0,
            subPositionUnit: 0,
            positionNonce: Math.floor(Math.random() * 255), // Pick a random nonce to force a position reset on start
            minPosition: 0,
            maxPosition: 4,
            positionWidthRadians: (10 * Math.PI) / 180,
            snapPoint: 1.1,
            text: 'From TS!',
        }),
    )
}

main()

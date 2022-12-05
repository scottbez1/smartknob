import React, { useEffect, useState } from 'react'
import './App.css'
import io from 'socket.io-client';
import Typography from '@mui/material/Typography';
import Container from '@mui/material/Container';
import ToggleButton from '@mui/material/ToggleButton'
import ToggleButtonGroup from '@mui/material/ToggleButtonGroup'
import { PB } from "smartknobjs-proto";


const socket = io();

enum Mode {
  Scroll = "Scroll",
  Frames = "Frames",
  Speed = "Speed",
}

const CONFIG_BY_MODE: Record<Mode, PB.SmartKnobConfig> = {
  [Mode.Scroll]: PB.SmartKnobConfig.create({
    numPositions: 32,
    position: 0,
    positionWidthRadians: 7*Math.PI / 180,
    detentStrengthUnit: 2.5,
    endstopStrengthUnit: 1,
    snapPoint: 0.7,
    text: "Scroll",
    detentPositions: [2, 10, 21, 22],
    snapPointBias: 0,
    snapPointBiasCenterPosition: 0,
  }),
  [Mode.Frames]: PB.SmartKnobConfig.create({
    numPositions: 256,
    position: 127,
    positionWidthRadians: 1*Math.PI / 180,
    detentStrengthUnit: 1,
    endstopStrengthUnit: 1,
    snapPoint: 1.1,
    text: "Frames",
    detentPositions: [],
    snapPointBias: 0,
    snapPointBiasCenterPosition: 0,
  }),
  [Mode.Speed]: PB.SmartKnobConfig.create({
    numPositions: 13,
    position: 6,
    positionWidthRadians: 60 * Math.PI / 180,
    detentStrengthUnit: 1,
    endstopStrengthUnit: 1,
    snapPoint: 0.55,
    text: "Playback\nSpeed",
    detentPositions: [],
    snapPointBias: 0.4,
    snapPointBiasCenterPosition: 6,
  }),
}

function App() {
  const [isConnected, setIsConnected] = useState(socket.connected);
  const [state, setState] = useState<string>("");

  const [mode, setMode] = useState<Mode>(Mode.Scroll);

  useEffect(() => {
    socket.on('connect', () => {
      setIsConnected(true);
    });

    socket.on('disconnect', () => {
      setIsConnected(false);
    });

    socket.on('state', (input: {pb: PB.SmartKnobState}) => {
      const {pb: state} = input
      const stateObj = PB.SmartKnobState.toObject(
        state,
        { defaults: true }
      );
      setState(JSON.stringify(stateObj, undefined, 4));
    });

    return () => {
      socket.off('connect');
      socket.off('disconnect');
      socket.off('state');
    };
  }, []);
    return (
      <Container component="main" maxWidth="xs">
        <Typography component="h1" variant="h5">
          Video Playback Control Demo
        </Typography>
        <ToggleButtonGroup
          color="primary"
          value={mode}
          exclusive
          onChange={(e, value: Mode) => {
            setMode(value)
            socket.emit("set_config", CONFIG_BY_MODE[value])
          }}
          aria-label="Mode"
        >
          {
            Object.keys(Mode).map((mode) => (
              <ToggleButton value={mode} key={mode}>{mode}</ToggleButton>
            ))
            
          }
        </ToggleButtonGroup>
        <div>
        {
          isConnected ? (
            <code>{state}</code>
          ) : <p>Not connected</p>
        }
        </div>
      </Container>
    )
}

export default App

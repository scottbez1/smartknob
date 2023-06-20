import React from 'react'
import ReactDOM from 'react-dom/client'
import {App} from './App'
import '@fontsource/roboto/300.css'
import '@fontsource/roboto/400.css'
import '@fontsource/roboto/500.css'
import '@fontsource/roboto/700.css'
import CssBaseline from '@mui/material/CssBaseline'
import {createTheme, ThemeProvider} from '@mui/material/styles'
import {VideoInfo} from './types'

const theme = createTheme()

const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement)

const info: VideoInfo = {
    totalFrames: 30 * 60 * 5,
    frameRate: 30,
    boundaryFrames: [312, 400, 1234, 1290, 3000, 3300, 4000, 8000, 8100],
}
root.render(
    <React.StrictMode>
        <ThemeProvider theme={theme}>
            <CssBaseline />
            <App info={info} />
        </ThemeProvider>
    </React.StrictMode>,
)

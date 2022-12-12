import React from 'react'
import ReactDOM from 'react-dom/client'
import './index.css'
import {App} from './App'
import reportWebVitals from './reportWebVitals'
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
    boundaryFrames: [312, 400, 1234, 1290, 3000, 4000],
}
root.render(
    <React.StrictMode>
        <ThemeProvider theme={theme}>
            <CssBaseline />
            <App info={info} />
        </ThemeProvider>
    </React.StrictMode>,
)

// If you want to start measuring performance in your app, pass a function
// to log results (for example: reportWebVitals(console.log))
// or send to an analytics endpoint. Learn more: https://bit.ly/CRA-vitals
reportWebVitals()

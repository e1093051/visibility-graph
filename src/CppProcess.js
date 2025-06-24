import { spawn } from 'child_process'
import path from 'path'
import { fileURLToPath } from 'url'

import { getPoint } from './PointMap.js'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)
const cppPath = path.join(__dirname, 'cpp', 'build', 'main')

class CppProcess {
  constructor() {
    this.proc = null
    this.buffer = ''
    this.queue = []
  }

  /**
 * 啟動 CPP 並初始化 arrangement
 * @param {Array<{x: number, y: number}>} points
 * @returns {Promise<void>}
 */
startWithPoints(points) {
  const input = [`${points.length}\n`, ...points.map(p => `${p.x} ${p.y}\n`)].join('')
  console.log(input)
  console.log("startWithPoints called")
  if (this.proc) {
    console.warn('Cpp process already started.')
    return Promise.resolve()
  }

  return new Promise((resolve, reject) => {
    this.proc = spawn(cppPath)
    this.proc.stdin.setDefaultEncoding('utf-8')
    this.proc.stdout.setEncoding('utf-8')

    let buffer = ''

    this.proc.stdout.on('data', data => {
      buffer += data.toString()
      // 檢查是否已經初始化完成
      console.log("Check if done")
      if (buffer.includes('INIT_DONE')) {
        console.log("Init Done!")
        this.proc.stdout.removeAllListeners('data')
        this.proc.stdout.on('data', d => this._handleOutput(d)) // 後續正常處理 query
        resolve()
      }
    })

    this.proc.stderr.on('data', err => console.error('[CPP stderr]', err.toString()))
    this.proc.on('exit', code => {
      console.log('[CPP exited]', code)
      this.proc = null
    })

    // 初始輸入：傳入點給 C++
    const input = [`${points.length}\n`, ...points.map(p => `${p.x} ${p.y}\n`)].join('')
    this.proc.stdin.write(input)
  })
}




query(refPoint) {
  return new Promise(resolve => {
    const handler = text => {
        return text
        .split('\n')
        .map(line => line.trim())
        .filter(Boolean)
        .map(line => {
        const [x, y] = line.split(' ').map(Number)
        if (isNaN(x) || isNaN(y)) {
            throw new Error(`Invalid point returned from C++: "${line}"`)
        }
        const point = getPoint(x, y)
        if (!point) {
            throw new Error(`Point (${x}, ${y}) not found in PointMap`)
        }
        return point
        })
    }
    this.queue.push([handler, resolve])
    this.proc.stdin.write(`QUERY\n${refPoint.x} ${refPoint.y}\n`)
  })
}


  _handleOutput(data) {
    this.buffer += data
    if (this.buffer.includes('END_OUTPUT\n')) {
      const [handler, resolve] = this.queue.shift()
      const result = this.buffer.replace('END_OUTPUT\n', '').trim()
      this.buffer = ''
      resolve(handler(result))
    }
  }

  kill() {
    if (this.proc) {
      this.proc.stdin.write('END\n')
      this.proc.kill()
      this.proc = null
    }
  }
  terminate() {
    if (this.proc) {
      this.proc.kill('SIGTERM') // 或 'exit' / 'SIGINT'
      this.proc = null
    }
  }
}
  
export const cppProcess = new CppProcess()
// cppInterface.js

import { getPoint } from './PointMap.js'
import { execSync } from 'child_process'
import path from 'path'
import { fileURLToPath } from 'url'


// todo: check if need to rebuild/amend current arrangement

// how to implement online update

export function getSortedPointsFromCpp(refPoint, points, isArrChanged=true) {
  let input = []
  const existing = getPoint(refPoint.x, refPoint.y)
  if (!existing) {
    throw new Error(`Reference point (${refPoint.x}, ${refPoint.y}) not found in PointMap`)
  }
  if (isArrChanged) {
    input = [points.length + '\n']
    for (const pt of points) {
        input.push(`${pt.x} ${pt.y}\n`)
    }
  }
  input.push(`QUERY\n${refPoint.x} ${refPoint.y}\n`)


  // 等同於 CommonJS 的 __filename 和 __dirname
  const __filename = fileURLToPath(import.meta.url)
  const __dirname = path.dirname(__filename)
  // 拼出正確的 cpp build 執行檔路徑
  const cppExecutablePath = path.join(__dirname, 'cpp', 'build', 'main')
  console.log(input)
  const output = execSync(cppExecutablePath, { input: input.join('') }).toString()

  const lines = output.trim().split('\n')

  // console.log(lines)

  return lines.map(line => {
    const [x, y] = line.trim().split(' ').map(Number)
    if (isNaN(x) || isNaN(y)) {
        throw new Error(`Invalid point returned from C++: "${line}"`)
    }
    const point = getPoint(x, y)
    // console.log(point);

    if (!point) throw new Error(`Point (${x}, ${y}) not found in PointMap`)
    return point
  })
}


import Point from './Point.js'  // ✅ Make sure to adjust the path as needed

const pointMap = new Map()

export function keyFromPoint(point) {
    return `${point.x.toFixed(8)},${point.y.toFixed(8)}`
}
  

export function keyFromXY(x, y) {
    return `${x.toFixed(8)},${y.toFixed(8)}`
}

export function registerPoint(point) {
    if (!(point instanceof Point)) {
        throw new Error('registerPoint expects a Point object')
    }
    const key = keyFromPoint(point)
    pointMap.set(key, point)
}

export function registerPoints(points) {
    for (const point of points) {
        // console.log('Registering:', point)
        registerPoint(point)
    }
}

export function getPoint(x, y) {
    const key = keyFromXY(x, y)
    return pointMap.get(key)
}

export function clearPointMap() {
    pointMap.clear()
}
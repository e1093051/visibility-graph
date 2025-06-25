import AVLTree from 'avl';

export default class EdgeKeys {

  constructor () {
    // this.keys = []
    
  const edgeKeyComparator = (a, b) => {
    if (a.matchesOtherKey(b)) return 0

    const ab = a.isLessThanOtherEdgeKey(b)
    const ba = b.isLessThanOtherEdgeKey(a)

    if (ab && ba) {
        // fallback 排序：根據 edge 的起點座標
        const ax = a.edge.p1.x, ay = a.edge.p1.y
        const bx = b.edge.p1.x, by = b.edge.p1.y
        return ax - bx || ay - by || a.edge.p2.x - b.edge.p2.x || a.edge.p2.y - b.edge.p2.y
    }

    if (!ab && !ba) {
        return 0 // 視為等價
    }

    return ab ? -1 : 1
  }
  
    this.keys = new AVLTree(edgeKeyComparator)
  }

  findKeyPosition (edgekey, p) {
    let lo = 0
    let hi = this.keys.length
    while (lo < hi) {
      const mid = Math.floor((lo + hi) / 2)
      if (edgekey.isLessThanOtherEdgeKey(this.keys[mid])) hi = mid
      else lo = mid + 1
    }
    return lo
  }

  addKey (edgekey, p) {
    /*
    const lo = this.findKeyPosition(edgekey)
    this.keys.splice(lo, 0, edgekey)
    */
    this.keys.insert(edgekey, edgekey)
  }
}

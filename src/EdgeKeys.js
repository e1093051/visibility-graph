import AVLTree from 'avl';

export default class EdgeKeys {

  constructor () {
    // this.keys = []
    
  const comparator = (a, b) => {
    const comparePoints = (p1, p2) => {
      if (p1.x !== p2.x) return p1.x - p2.x
      return p1.y - p2.y
    }
  
    const [a1, a2] = comparePoints(a.edge.p1, a.edge.p2) <= 0 ? [a.edge.p1, a.edge.p2] : [a.edge.p2, a.edge.p1]
    const [b1, b2] = comparePoints(b.edge.p1, b.edge.p2) <= 0 ? [b.edge.p1, b.edge.p2] : [b.edge.p2, b.edge.p1]
  
    const cmp1 = comparePoints(a1, b1)
    if (cmp1 !== 0) return cmp1
    return comparePoints(a2, b2)
  }
    
      
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
  
    this.keys = new AVLTree(comparator)
  }

  addKey (edgekey, p) {
    /*
    const lo = this.findKeyPosition(edgekey)
    this.keys.splice(lo, 0, edgekey)
    */
    this.keys.insert(edgekey, edgekey)
  }
}

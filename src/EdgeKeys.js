import AVLTree from 'avl';

export default class EdgeKeys {
  constructor() {
    // Store insertion order separately
    this.insertionOrders = new Map(); // Uses EdgeKeys as keys
    this.counter = 0;
    this.keys = new AVLTree(this.createComparator());
  }

  createComparator() {
    return (a, b) => {
      //console.log('Compare:', a.edge.p1.nodeId, '-', a.edge.p2.nodeId, 'vs', b.edge.p1.nodeId, '-', b.edge.p2.nodeId);
      if (a.matchesOtherKey(b)) return 0;
  
      const aLessThanB = a.isLessThanOtherEdgeKey(b);
      const bLessThanA = b.isLessThanOtherEdgeKey(a);
  
      if (aLessThanB && !bLessThanA) return -1;
      if (!aLessThanB && bLessThanA) return 1;
  
      // Conflict: use insertion order as stable tiebreaker
      //console.log("Insertion Order matters")
      const orderA = this.insertionOrders.get(a.toString());
      const orderB = this.insertionOrders.get(b.toString());
      return orderB - orderA;
    };
  }


  addKey(edgekey) {
    /*
    if (!this.insertionOrders.has(edgekey)) {
      this.insertionOrders.set(edgekey, this.counter++);
    }
      */
    this.insertionOrders.set(edgekey.toString(), this.counter++);
    this.keys.insert(edgekey);
  }

}

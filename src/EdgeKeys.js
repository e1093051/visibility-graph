import AVLTree from 'avl';

export default class EdgeKeys {
  constructor() {
    // Store insertion order separately
    this.insertionOrders = new WeakMap(); // Uses EdgeKeys as keys
    this.counter = 0;
    this.keys = new AVLTree(this.createComparator());
  }

  createComparator() {
    return (a, b) => {
      if (a.matchesOtherKey(b)) return 0;
      
      const aLessThanB = a.isLessThanOtherEdgeKey(b);
      const bLessThanA = b.isLessThanOtherEdgeKey(a);

      if ((aLessThanB && bLessThanA) || (!aLessThanB && !bLessThanA)) {
        // Get stored insertion orders
        const orderA = this.insertionOrders.get(a);
        const orderB = this.insertionOrders.get(b);
        return orderA - orderB; // Earlier insertions come first
      }

      return aLessThanB ? -1 : 1;
    };
  }
  

  addKey(edgekey) {
    if (!this.insertionOrders.has(edgekey)) {
      this.insertionOrders.set(edgekey, this.counter++);
    }
    this.keys.insert(edgekey);
  }
}

Array.prototype.map = function (callback) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    r.push(callback(this[i]));
    i = i + 1;
  }
  return r;
};

Array.prototype.sort = function(compareFn) {
  if (compareFn === undefined) {
    compareFn = function (a, b) {
      return a - b;
    };
  }
  var i, j;
  for (i = 0; i < this.length - 1; i++) {
    for (j = i + 1; j < this.length; j++) {
      if (compareFn(this[i], this[j]) > 0) {
        var t = this[i];
        this[i] = this[j];
        this[j] = t;
      }
    }
  }
  return this;
};


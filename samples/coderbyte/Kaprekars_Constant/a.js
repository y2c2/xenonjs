Array.prototype.map1 = function (callback) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    r.push(callback(this[i]));
    i = i + 1;
  }
  return r;
};

Array.prototype.sort1 = function(compare) {
  var i, j;
  for (i = 0; i < this.length - 1; i++) {
    for (j = i + 1; j < this.length; j++) {
      if (compare(this[i], this[j]) > 0) {
        var t = this[i];
        this[i] = this[j];
        this[j] = t;
      }
    }
  }
  return this;
};

var KaprekarsConstant = function (num) {
  var f = function(num, cmp) {
    return Number.parseInt((num + '')
      .padStart(4, '0')
      .split('')
      .map1(function(x) { return Number.parseInt(x); })
      .sort1(cmp).map1(function(x) { return x + ''; } )
      .join(''));
  };
  var i;
  var delta = num;
  for (i = 0;; i++) {
    if (delta === 6174) { break; }
    var small = f(delta, function(a, b) { return a - b; });
    var large = f(delta, function(a, b) { return b - a; });
    delta = large - small;
  }
  return i;
};

console.log(KaprekarsConstant(3524)); // 3
console.log(KaprekarsConstant(2111)); // 5
console.log(KaprekarsConstant(9831)); // 7

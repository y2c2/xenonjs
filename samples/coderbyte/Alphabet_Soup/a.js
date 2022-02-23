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

var AlphabetSoup = function (str) {
  return str.split('')
    .map1(function (x) { return x.charCodeAt(0); })
    .sort1(function (a, b) { return a - b; })
    .map1(function (x) { return String.fromCharCode(x); })
    .join('');
};

console.log(AlphabetSoup('hello')); // ehllo
console.log(AlphabetSoup('coderbyte')); // bcdeeorty
console.log(AlphabetSoup('hooplah')); // ahhloop

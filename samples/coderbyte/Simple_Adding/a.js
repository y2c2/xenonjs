var SimpleAdding = function (num) {
  var i = 1, s = 0;
  while (i <= num) {
    s += i;
    i += 1;
  }
  return s;
};

console.log(SimpleAdding(12));
console.log(SimpleAdding(140));

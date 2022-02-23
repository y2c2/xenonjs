Array.prototype.map1 = function (callback) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    r.push(callback(this[i]));
    i = i + 1;
  }
  return r;
};

var LetterCapitalize = function (str) {
  return str.split(' ').map1(function (word) {
    var first = word.charAt(0);
    if ((97 <= first.charCodeAt(0) && first.charCodeAt(0) <= 122)) first = String.fromCharCode(first.charCodeAt(0) - 32);
    return first + word.substr(1);
  }).join(' ');
};

console.log(LetterCapitalize("hello world"));
console.log(LetterCapitalize("i ran there"));

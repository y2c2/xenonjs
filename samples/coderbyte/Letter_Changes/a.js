Array.prototype.map1 = function (callback) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    r.push(callback(this[i]));
    i = i + 1;
  }
  return r;
};

var LetterChanges = function(str) { 
  return str.split('').map1(function (c) {
    var code = c.charCodeAt(0);
    if (97 <= code && code <= 122) {
      var c1 = c == 'z' ? 'a' : String.fromCharCode(code + 1);
      if (c1 == 'a') c1 = 'A';
      else if (c1 == 'e') c1 = 'E';
      else if (c1 == 'i') c1 = 'I';
      else if (c1 == 'o') c1 = 'O';
      else if (c1 == 'u') c1 = 'U';
      return c1;
    }
    return c;
  }).join('');
};

console.log(LetterChanges('hello*3'));
console.log(LetterChanges('fun times!'));

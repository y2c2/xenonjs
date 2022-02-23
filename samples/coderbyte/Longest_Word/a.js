Array.prototype.map1 = function (callback) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    r.push(callback(this[i]));
    i = i + 1;
  }
  return r;
};

Array.prototype.filter1 = function (test) {
  var i = 0;
  var r = [];
  while (i < this.length) {
    if (test(this[i])) {
      r.push(this[i]);
    }
    i = i + 1;
  }
  return r;
};

var LongestWord = function (sen) {
  var words = sen.split(' ');
  words = words.map1(function(word) {
    return word.split('').filter1(function (ch) {
      return (97 <= ch.charCodeAt(0) && ch.charCodeAt(0) <= 122) ||
        (65 <= ch.charCodeAt(0) && ch.charCodeAt(0) <= 90);
    }).join('');
  });
  var i = 1, m = 0;
  while (i < words.length) {
    if (words[i].length > words[m].length) {
      m = i;
    }
    i += 1;
  }
  return words[m];
};

console.log(LongestWord('fun&!! time'));
console.log(LongestWord('I love dogs'));

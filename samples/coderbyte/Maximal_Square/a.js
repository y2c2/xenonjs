var test = function(strArr, x, y, size) {
  var i, j;
  for (j = y; j < y + size; j++) {
    for (i = x; i < x + size; i++) {
      if (strArr[j][i] == '0') return false;
    }
  }
  return true;
};

var MaximalSquare = function (strArr) {
  var height = strArr.length, width = strArr[0].length;
  var maximumSize, x, y;
  for (maximumSize = Math.min(height, width); maximumSize >= 1; maximumSize--) {
    for (x = 0; x <= width - maximumSize; x++) {
      for (y = 0; y <= height - maximumSize; y++) {
        if (test(strArr, x, y, maximumSize)) {
          return maximumSize * maximumSize;
        }
      }
    }
  }
  return 0;
};
   
console.log(MaximalSquare(["10100", "10111", "11111", "10010"])); // 4
console.log(MaximalSquare(["0111", "1111", "1111", "1111"])); // 9
console.log(MaximalSquare(["0111", "1101", "0111"])); // 1


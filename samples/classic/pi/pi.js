var calcPi = function() {
  var n = 50;
  var pi = 0;
  var i = 0;
  while (i < n) {
    var temp = 4 / (i*2+1);
    if (i % 2 == 0) {
      pi += temp;
    }
    else {
      pi -= temp;
    }

    i += 1;
  }
  return pi;
};

console.log(calcPi());

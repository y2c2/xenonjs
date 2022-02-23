var gcd = function (x, y) {
  while (y != 0) {
    var t = y;
    y = x % y;
    x = t;
  }
  return x;
};

var lcm = function (arr) {
  var i = 1, a = Math.abs(arr[0]);
  while(i < arr.length) {
    a = a / gcd(a, arr[i]) * arr[i];
    i += 1;
  }
  return a;
};

console.log(lcm([12, 18])); // 36
console.log(lcm([21, 35])); // 105

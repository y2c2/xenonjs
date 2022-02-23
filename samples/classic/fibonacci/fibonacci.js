var fib = function (n) {
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
};

var i = 0;
while (i <= 7) {
  console.log(fib(i));
  i = i + 1;
};

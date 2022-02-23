var fizzBuzz = function () {
  var i, output;
  i = 1;
  while (i < 101) {
    output = '';
    if (!(i % 3)) { output += 'Fizz'; }
    if (!(i % 5)) { output += 'Buzz'; }
    console.log(output || i);
    i += 1;
  };
};
fizzBuzz();

var factors = function (num) {
  var n_factors = [], i;
  i = 1;
  while (i <= Math.floor(Math.sqrt(num))) {
    if (num % i === 0) {
      n_factors.push(i);
      if (num / i !== i) {
        n_factors.push(num / i);
      }
    }
    i = i + 1;
  };
  n_factors.sort();
  return n_factors;
};

console.log(factors(45));  // [1,3,5,9,15,45] 
console.log(factors(53));  // [1,53] 
console.log(factors(64));  // [1,2,4,8,16,32,64]

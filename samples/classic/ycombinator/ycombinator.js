var y = function(le) {
  return function(f) { 
    return f(f); 
  }(function(f) {
    return le(
      function(x) { return (f(f))(x); }
    );
  });
};

var factgen = function(fact) {
  return function(n) {
    return (n === 0) ? 1 : n * fact(n-1);
  };
};

console.log(y(factgen)(10));

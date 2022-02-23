var Person = function (name, age) {
  inspect(name);
  inspect(age);
  // console.log(name);
  // console.log(age);
  // this.name = name;
  // this.age = age;
};

var createPerson = function(name, age) {
  return new Person(name, age);
};

var p = createPerson('Jack', 15);
console.log(p);

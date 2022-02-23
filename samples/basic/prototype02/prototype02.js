var Person = function (name) {
  this.name = name;
};
Person.prototype.greet = function () {
  console.log("Hello, " + this.name);
};
var p = new Person("Jack");
p.greet();


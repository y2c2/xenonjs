// Object
console.log(Object.prototype.__proto__ == null); // true
console.log(({}).__proto__ === Object.prototype);  // true
var obj = {"key": "value"};
console.log(obj.__proto__ === Object.prototype); // true
console.log(obj.__proto__.__proto__ === null); // true

// Function
console.log(Function.prototype.__proto__ === Object.prototype); // true
var f = function() {};
console.log(f.__proto__ === Function.prototype); // true
console.log(f.__proto__.__proto__ === Object.prototype); // true
console.log(f.__proto__.__proto__.__proto__ === null); // true

// String
console.log(String.prototype.__proto__ === Object.prototype); // true
var s = "";
console.log(s.__proto__ === String.prototype); // true
console.log(s.__proto__.__proto__ === Object.prototype); // true
console.log(s.__proto__.__proto__.__proto__ === null); // true

// Array
console.log(Array.prototype.__proto__ === Object.prototype); // true
var a = [];
console.log(a.__proto__ === Array.prototype); // true
console.log(a.__proto__.__proto__ === Object.prototype); // true
console.log(a.__proto__.__proto__.__proto__ === null); // true

// JSON
console.log(JSON.__proto__ === Object.prototype); // true
console.log(JSON.__proto__.__proto__ === null); // true

// Number
console.log(Number.prototype.__proto__ === Object.prototype); // true

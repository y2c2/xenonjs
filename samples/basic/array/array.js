var a = [];
console.log(a);
console.log(a.length);
a = [1];
console.log(a);
console.log(a.length);
a = [1,2];
console.log(a);
console.log(a.length);

// push & shift
a = [];
a.push(1);
console.log(a); // [1]
a.push(2);
console.log(a); // [1,2]
a.push(3);
console.log(a); // [1,2,3]
console.log(a.shift()); // 1
console.log(a); // [2,3]
console.log(a.shift()); // 2
console.log(a); // [3]
console.log(a.shift()); // 3
console.log(a); // []
console.log(a.shift()); // undefined
console.log(a); // []

// unshift & pop
a = [];
a.unshift(3);
console.log(a); // [3]
a.unshift(2);
console.log(a); // [2,3]
a.unshift(1);
console.log(a); // [1,2,3]
console.log(a.pop()); // 3
console.log(a); // [1,2]
console.log(a.pop()); // 2
console.log(a); // [1]
console.log(a.pop()); // 1
console.log(a); // []
console.log(a.pop()); // undefined
console.log(a); // []

// []
var a = [1,2,3];
console.log(a[-1]); // undefined
console.log(a[0]); // 1
console.log(a[1]); // 2
console.log(a[2]); // 3
console.log(a[3]); // undefined
a[0] = 4;
console.log(a); // [4,2,3]


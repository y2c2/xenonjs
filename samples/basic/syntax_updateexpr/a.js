var a = 123;

console.log(a++); // 123
console.log(a); // 124

console.log(a--); // 124
console.log(a); // 123

console.log(++a); // 124
console.log(a); // 124

console.log(--a); // 123
console.log(a); // 123

var arr = [1, 2, 3];

console.log(arr[0]++); // 1
console.log(arr[1]++); // 2
console.log(arr[2]++); // 3
console.log(arr); // [2,3,4]

console.log(arr[0]--); // 2
console.log(arr[1]--); // 3
console.log(arr[2]--); // 4
console.log(arr); // [1,2,3]

console.log(++arr[0]); // 2
console.log(++arr[1]); // 3
console.log(++arr[2]); // 4
console.log(arr); // [2,3,4]

console.log(--arr[0]); // 1
console.log(--arr[1]); // 2
console.log(--arr[2]); // 3
console.log(arr); // [1,2,3]

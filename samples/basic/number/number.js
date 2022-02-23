// The following examples all return 15:

console.log(Number.parseInt(' 0xF', 16));
console.log(Number.parseInt(' F', 16));
console.log(Number.parseInt('17', 8));
console.log(Number.parseInt(021, 8));   // 021 -> 17 -> "17" -> 15
console.log(Number.parseInt('015', 10));   // parseInt(015, 10); will return 15
console.log(Number.parseInt(15.99, 10));
console.log(Number.parseInt('15,123', 10));
console.log(Number.parseInt('FXX123', 16));
console.log(Number.parseInt('1111', 2));
console.log(Number.parseInt('15 * 3', 10));
console.log(Number.parseInt('15e2', 10));
console.log(Number.parseInt('15px', 10));
console.log(Number.parseInt('12', 13));

// The following examples all return undefined:

console.log(Number.parseInt('Hello', 8)); // Not a number at all
console.log(Number.parseInt('546', 2));   // Digits are not valid for binary representations

// The following examples all return -15:

console.log(Number.parseInt('-F', 16));
console.log(Number.parseInt('-0F', 16));
console.log(Number.parseInt('-0XF', 16));
console.log(Number.parseInt(-15.1, 10));
console.log(Number.parseInt(' -17', 8));
console.log(Number.parseInt(' -15', 10));
console.log(Number.parseInt('-1111', 2));
console.log(Number.parseInt('-15e1', 10));
console.log(Number.parseInt('-12', 13));

console.log(Number.parseInt('0e0', 16)); // 224

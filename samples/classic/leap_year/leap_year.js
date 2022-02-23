var isLeapYear = function (year) {
  return (year % 100 === 0) ? (year % 400 === 0) : (year % 4 === 0);
};

console.log(isLeapYear(2400)); // true
console.log(isLeapYear(2012)); // true
console.log(isLeapYear(2000)); // true
console.log(isLeapYear(1600)); // true
console.log(isLeapYear(1500)); // false
console.log(isLeapYear(1400)); // false

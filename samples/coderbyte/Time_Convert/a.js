var TimeConvert = function (num) {
  return Number.parseInt(num / 60) + ':' + num % 60;
};

console.log(TimeConvert(63));
console.log(TimeConvert(126));
console.log(TimeConvert(45));

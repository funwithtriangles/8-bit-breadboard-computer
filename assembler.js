const fs = require('fs')
const path = require('path')
const commands = require('./commands')

const args = process.argv.slice(2)
const inputFile = args[0]
const fileName = path.basename(inputFile)
const outputDir = args[1]

let currLine = 0;

const dec2bin = (n) => {
  n = parseFloat(n)
  if (n < 0 || n > 255 || n % 1 !== 0) {
      throw new Error(`${n} does not fit in a byte. Line ${currLine}`)
  }
  return ("000000000" + n.toString(2)).substr(-8)
}

const shouldSkip = line => 
  line.length <= 0 ||
  line.trim()[0] === ';'

const processLine = line => {
  return {
    line,
    firstChar: line.substr(0, 1),
    lastChar: line.substr(-1),
  }
}

const processNum = (num, l) => 
  num.substr(0, 1) === 'b' ?
  num.substr(1) :
  dec2bin(num, l)

const labels = {}
const vars = {}

// Split by lines, remove comments, then split by spaces
const rawLines = fs.readFileSync(inputFile).toString().split('\n');
const lines = []
rawLines.forEach(rl => {
  if (shouldSkip(rl)) return
  rl = rl.split(';')[0].trim()
  rl.split(' ').forEach(l => {
    if (l.length > 0) {
      lines.push(l)
    }
  })
})

// Find labels and variables
lines.forEach(l => {
  const { line, firstChar, lastChar } = processLine(l)

  if (lastChar === ':') {
    const labelName = line.substr(0, line.length - 1)
    labels[labelName] = dec2bin(currLine)
  } else if (firstChar === '$') {
    const lineParts = line.split('=')
    const varName = lineParts[0].substr(1)
    vars[varName] = processNum(lineParts[1])
  } else {
    currLine ++
  }
})

console.log(labels, vars)

let assembled = []

// Assemble
lines.forEach((l, i) => {
  const { line, firstChar, lastChar } = processLine(l)

  console.log(i, l)

  // If label or var, ignore
  if (lastChar === ':' || firstChar === '$') return

  let bin = labels[line] ?? vars[line] ?? commands[line]

  if (bin === undefined) {
    bin = processNum(line)
  }

  assembled.push(`0b${bin},`)
})

fs.writeFileSync(
  `${outputDir}/${fileName}`,
  assembled.join('\n')
)

console.log(`Done: ${outputDir}/${fileName}`)
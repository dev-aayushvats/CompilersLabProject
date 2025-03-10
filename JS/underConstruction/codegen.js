class CodeGenerator {
  constructor() {
    this.js = [];
  }

  generate(ast) {
    this.generateNode(ast);
    const jsCode = this.js.join('\n');
    console.log('Output from Code Generation stage:', jsCode);
    return jsCode;
  }

  generateNode(ast) {
    switch (ast.type) {
      case 'PROGRAM':
      case 'BLOCK':
        ast.children.forEach(child => this.generateNode(child));
        break;
      case 'DECLARATION':
        this.js.push(`let ${ast.value} = ${this.generateExpression(ast.children[0])};`);
        break;
      case 'ASSIGNMENT':
        this.js.push(`${ast.value} = ${this.generateExpression(ast.children[0])};`);
        break;
      case 'FOR':
        const [init, cond, update, body] = ast.children;
        const updateExpr = update.type === 'ASSIGNMENT' 
          ? `${update.value} = ${this.generateExpression(update.children[0])}` 
          : this.generateExpression(update);
        this.js.push(`for (${this.generateNode(init).slice(0, -1)} ${this.generateExpression(cond)}; ${updateExpr}) {`);
        this.generateNode(body);
        this.js.push('}');
        break;
      case 'WHILE':
        this.js.push(`while (${this.generateExpression(ast.children[0])}) {`);
        this.generateNode(ast.children[1]);
        this.js.push('}');
        break;
      case 'DO_WHILE':
        this.js.push('do {');
        this.generateNode(ast.children[0]);
        this.js.push(`} while (${this.generateExpression(ast.children[1])});`);
        break;
      case 'IF':
        this.js.push(`if (${this.generateExpression(ast.children[0])}) {`);
        this.generateNode(ast.children[1]);
        if (ast.children[2]) {
          this.js.push('} else {');
          this.generateNode(ast.children[2]);
        }
        this.js.push('}');
        break;
      case 'SWITCH':
        this.js.push(`switch (${this.generateExpression(ast.value)}) {`);
        ast.children.forEach(caseNode => {
          if (caseNode.type === 'CASE') {
            this.js.push(`case ${this.generateExpression(caseNode.value)}:`);
            caseNode.children.forEach(stmt => this.generateNode(stmt));
          } else if (caseNode.type === 'DEFAULT') {
            this.js.push('default:');
            caseNode.children.forEach(stmt => this.generateNode(stmt));
          }
        });
        this.js.push('}');
        break;
      case 'BREAK':
        this.js.push('break;');
        break;
    }
  }

  generateExpression(ast) {
    if (ast.type === 'NUMBER' || ast.type === 'IDENTIFIER') return ast.value;
    if (ast.type === 'BINOP') {
      const left = this.generateExpression(ast.children[0]);
      const right = this.generateExpression(ast.children[1]);
      return `(${left} ${ast.value} ${right})`;
    }
    if (ast.type === 'COMPARE') {
      const left = this.generateExpression(ast.children[0]);
      const right = this.generateExpression(ast.children[1]);
      return `${left} ${ast.value} ${right}`;
    }
    return '';
  }
}

module.exports = CodeGenerator;
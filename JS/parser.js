const ASTNode = require('./underConstruction/ast');

class Parser {
  constructor(tokens) {
    this.tokens = tokens;
    this.pos = 0;
  }

  parse() {
    const statements = [];
    while (this.pos < this.tokens.length) {
      statements.push(this.parseStatement());
    }
    const ast = new ASTNode('PROGRAM', null, statements);
    console.log('Output from Parser stage:', JSON.stringify(ast, null, 2));
    return ast;
  }

  consume(type) {
    if (this.tokens[this.pos].type === type) this.pos++;
    else throw new Error(`Expected ${type}, got ${this.tokens[this.pos]?.value || 'EOF'}`);
  }

  parseStatement() {
    const token = this.tokens[this.pos];
    if (token.type === 'INT') return this.parseDeclaration();
    if (token.type === 'IDENTIFIER') return this.parseAssignment();
    if (token.type === 'FOR') return this.parseFor();
    if (token.type === 'WHILE') return this.parseWhile();
    if (token.type === 'DO') return this.parseDoWhile();
    if (token.type === 'IF') return this.parseIf();
    if (token.type === 'SWITCH') return this.parseSwitch();
    if (token.type === 'BREAK') {
      this.pos++;
      this.consume('SEMICOLON');
      return new ASTNode('BREAK', 'break');
    }
    if (token.type === 'LBRACE') return this.parseBlock();
    throw new Error(`Unexpected token: ${token.value}`);
  }

  parseDeclaration() {
    this.pos++; // Skip 'int'
    const varName = this.tokens[this.pos++].value;
    this.consume('EQUALS');
    const expr = this.parseExpression();
    this.consume('SEMICOLON');
    return new ASTNode('DECLARATION', varName, [expr]);
  }

  parseAssignment() {
    const varName = this.tokens[this.pos++].value;
    this.consume('EQUALS');
    const expr = this.parseExpression();
    this.consume('SEMICOLON');
    return new ASTNode('ASSIGNMENT', varName, [expr]);
  }

  parseExpression() {
    let node = this.parseTerm();
    while (this.pos < this.tokens.length && ['PLUS', 'MINUS'].includes(this.tokens[this.pos].type)) {
      const op = this.tokens[this.pos++].value;
      const right = this.parseTerm();
      node = new ASTNode('BINOP', op, [node, right]);
    }
    return node;
  }

  parseTerm() {
    let node = this.parseFactor();
    while (this.pos < this.tokens.length && ['MULT', 'DIV', 'MOD'].includes(this.tokens[this.pos].type)) {
      const op = this.tokens[this.pos++].value;
      const right = this.parseFactor();
      node = new ASTNode('BINOP', op, [node, right]);
    }
    return node;
  }

  parseFactor() {
    const token = this.tokens[this.pos];
    if (token.type === 'NUMBER') {
      this.pos++;
      return new ASTNode('NUMBER', token.value);
    }
    if (token.type === 'IDENTIFIER') {
      this.pos++;
      if (this.pos < this.tokens.length && this.tokens[this.pos].type === 'EQUALS') {
        this.pos++; // Skip '='
        const right = this.parseExpression();
        return new ASTNode('ASSIGNMENT', token.value, [right]);
      }
      return new ASTNode('IDENTIFIER', token.value);
    }
    if (token.type === 'LPAREN') {
      this.pos++;
      const expr = this.parseExpression();
      this.consume('RPAREN');
      return expr;
    }
    throw new Error(`Unexpected token in expression: ${token.value}`);
  }

  parseComparison() {
    const left = this.parseExpression();
    const op = this.tokens[this.pos];
    if (['LT', 'GT', 'LTE', 'GTE', 'EQ', 'NEQ'].includes(op.type)) {
      this.pos++;
      const right = this.parseExpression();
      return new ASTNode('COMPARE', op.value, [left, right]);
    }
    return left;
  }

  parseFor() {
    this.pos++; // Skip 'for'
    this.consume('LPAREN');
    const init = this.parseStatement();
    const condition = this.parseComparison();
    this.consume('SEMICOLON');
    const update = this.parseFactor();
    this.consume('RPAREN');
    const body = this.parseStatement();
    return new ASTNode('FOR', null, [init, condition, update, body]);
  }

  parseWhile() {
    this.pos++; // Skip 'while'
    this.consume('LPAREN');
    const condition = this.parseComparison();
    this.consume('RPAREN');
    const body = this.parseStatement();
    return new ASTNode('WHILE', null, [condition, body]);
  }

  parseDoWhile() {
    this.pos++; // Skip 'do'
    const body = this.parseStatement();
    this.consume('WHILE');
    this.consume('LPAREN');
    const condition = this.parseComparison();
    this.consume('RPAREN');
    this.consume('SEMICOLON');
    return new ASTNode('DO_WHILE', null, [body, condition]);
  }

  parseIf() {
    this.pos++; // Skip 'if'
    this.consume('LPAREN');
    const condition = this.parseComparison();
    this.consume('RPAREN');
    const thenBranch = this.parseStatement();
    let elseBranch = null;
    if (this.pos < this.tokens.length && this.tokens[this.pos].type === 'ELSE') {
      this.pos++; // Skip 'else'
      elseBranch = this.parseStatement();
    }
    return new ASTNode('IF', null, [condition, thenBranch, elseBranch].filter(Boolean));
  }

  parseSwitch() {
    this.pos++; // Skip 'switch'
    this.consume('LPAREN');
    const expr = this.parseExpression();
    this.consume('RPAREN');
    this.consume('LBRACE');
    const cases = [];
    while (this.pos < this.tokens.length && this.tokens[this.pos].type !== 'RBRACE') {
      if (this.tokens[this.pos].type === 'CASE') {
        this.pos++; // Skip 'case'
        const value = this.parseExpression();
        this.consume('COLON');
        const body = [];
        while (this.pos < this.tokens.length && !['CASE', 'DEFAULT', 'RBRACE'].includes(this.tokens[this.pos].type)) {
          body.push(this.parseStatement());
        }
        cases.push(new ASTNode('CASE', value, body));
      } else if (this.tokens[this.pos].type === 'DEFAULT') {
        this.pos++; // Skip 'default'
        this.consume('COLON');
        const body = [];
        while (this.pos < this.tokens.length && !['CASE', 'DEFAULT', 'RBRACE'].includes(this.tokens[this.pos].type)) {
          body.push(this.parseStatement());
        }
        cases.push(new ASTNode('DEFAULT', null, body));
      }
    }
    this.consume('RBRACE');
    return new ASTNode('SWITCH', expr, cases);
  }

  parseBlock() {
    this.consume('LBRACE');
    const statements = [];
    while (this.pos < this.tokens.length && this.tokens[this.pos].type !== 'RBRACE') {
      statements.push(this.parseStatement());
    }
    this.consume('RBRACE');
    return new ASTNode('BLOCK', null, statements);
  }
}

module.exports = Parser;
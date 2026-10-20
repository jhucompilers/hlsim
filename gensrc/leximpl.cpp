struct yylex_charclass {
  uint32_t bits[4];
};

static int yylex_is_member(const struct yylex_charclass *cc, int c) {
  int bit_offset, word_index;
  bit_offset = (c & 0x1F);
  c >>= 5;
  word_index = (c & 0x03);
  c >>= 2;
  if (c != 0)
    return 0;
  return (cc->bits[word_index] & (1U << bit_offset)) != 0U;
}

static const struct yylex_charclass yylex_cclass_0 = {
  { 0x0, 0x0, 0x87fffffe, 0x7bffffe, }
};
static const struct yylex_charclass yylex_cclass_1 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7f7ff7e, }
};
static const struct yylex_charclass yylex_cclass_2 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7fffffe, }
};
static const struct yylex_charclass yylex_cclass_3 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7fbfffe, }
};
static const struct yylex_charclass yylex_cclass_4 = {
  { 0x0, 0x4000, 0x87fffffe, 0x7fffffe, }
};
static const struct yylex_charclass yylex_cclass_5 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7ffeffe, }
};
static const struct yylex_charclass yylex_cclass_6 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7ffffde, }
};
static const struct yylex_charclass yylex_cclass_7 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7fffff6, }
};
static const struct yylex_charclass yylex_cclass_8 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7effffe, }
};
static const struct yylex_charclass yylex_cclass_9 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7fffdfe, }
};
static const struct yylex_charclass yylex_cclass_10 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7ff7ffe, }
};
static const struct yylex_charclass yylex_cclass_11 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7ffbffe, }
};
static const struct yylex_charclass yylex_cclass_12 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7fffffa, }
};
static const struct yylex_charclass yylex_cclass_13 = {
  { 0x0, 0x3ff4000, 0x87fffffe, 0x7ffeffc, }
};

uint8_t yylex_lookup_next_state(const uint8_t *table, int min_cc, int size, int c) {
  int index;
  index = c - min_cc;
  if (index < 0 || index >= size)
    return -1;
  else
    return table[index];
}

static const uint8_t yylex_lookup_0[] = {
  1, 255, 255, 255, 2, 3, 255, 255, 4, 255, 5, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 6, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 8,
};

static const int yylex_final_state_to_token_type[] = {
  -1,
  -1,
  (int) TokenKind::LPAREN,
  (int) TokenKind::RPAREN,
  (int) TokenKind::COMMA,
  (int) TokenKind::IDENT,
  (int) TokenKind::COLON,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::VREG,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::SECTION,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::IDENT,
  (int) TokenKind::GLOBL,
  (int) TokenKind::GLOBL,
  (int) TokenKind::IMMEDIATE,
};

static int yylex(LEXER_TYPE lexer, LEXEME_BUF_TYPE lexeme_buf) {
  uint8_t state = 0, next_state;
  int c;
  for (;;) {
    c = GET(lexer);
    if (c == EOF)
      break;
    next_state = -1;
    switch (state) {
    case 0:
      if (yylex_is_member(&yylex_cclass_0, c))
        next_state = 7;
      else
        next_state = yylex_lookup_next_state(yylex_lookup_0, 36, 83, c);
      break;
    case 1:
      if (isdigit(c))
        next_state = 26;
      break;
    case 5:
      if (yylex_is_member(&yylex_cclass_1, c))
        next_state = 9;
      else if (c == 'g')
        next_state = 12;
      else if (c == 's')
        next_state = 13;
      break;
    case 7:
      if (yylex_is_member(&yylex_cclass_2, c))
        next_state = 9;
      break;
    case 8:
      if (yylex_is_member(&yylex_cclass_3, c))
        next_state = 9;
      else if (c == 'r')
        next_state = 10;
      break;
    case 9:
      if (yylex_is_member(&yylex_cclass_2, c))
        next_state = 9;
      break;
    case 10:
      if (yylex_is_member(&yylex_cclass_4, c))
        next_state = 9;
      else if (isdigit(c))
        next_state = 11;
      break;
    case 11:
      if (yylex_is_member(&yylex_cclass_4, c))
        next_state = 9;
      else if (isdigit(c))
        next_state = 11;
      break;
    case 12:
      if (yylex_is_member(&yylex_cclass_5, c))
        next_state = 9;
      else if (c == 'l')
        next_state = 20;
      break;
    case 13:
      if (yylex_is_member(&yylex_cclass_6, c))
        next_state = 9;
      else if (c == 'e')
        next_state = 14;
      break;
    case 14:
      if (yylex_is_member(&yylex_cclass_7, c))
        next_state = 9;
      else if (c == 'c')
        next_state = 15;
      break;
    case 15:
      if (yylex_is_member(&yylex_cclass_8, c))
        next_state = 9;
      else if (c == 't')
        next_state = 16;
      break;
    case 16:
      if (yylex_is_member(&yylex_cclass_9, c))
        next_state = 9;
      else if (c == 'i')
        next_state = 17;
      break;
    case 17:
      if (yylex_is_member(&yylex_cclass_10, c))
        next_state = 9;
      else if (c == 'o')
        next_state = 18;
      break;
    case 18:
      if (yylex_is_member(&yylex_cclass_11, c))
        next_state = 9;
      else if (c == 'n')
        next_state = 19;
      break;
    case 19:
      if (yylex_is_member(&yylex_cclass_2, c))
        next_state = 9;
      break;
    case 20:
      if (yylex_is_member(&yylex_cclass_10, c))
        next_state = 9;
      else if (c == 'o')
        next_state = 21;
      break;
    case 21:
      if (yylex_is_member(&yylex_cclass_12, c))
        next_state = 9;
      else if (c == 'b')
        next_state = 22;
      break;
    case 22:
      if (yylex_is_member(&yylex_cclass_13, c))
        next_state = 9;
      else if (c == 'a')
        next_state = 23;
      else if (c == 'l')
        next_state = 24;
      break;
    case 23:
      if (yylex_is_member(&yylex_cclass_5, c))
        next_state = 9;
      else if (c == 'l')
        next_state = 25;
      break;
    case 24:
      if (yylex_is_member(&yylex_cclass_2, c))
        next_state = 9;
      break;
    case 25:
      if (yylex_is_member(&yylex_cclass_2, c))
        next_state = 9;
      break;
    case 26:
      if (isdigit(c))
        next_state = 26;
      break;
    } // end switch
    if (((int8_t) next_state) == -1) {
      UNGET(lexer, c);
      break;
    }
    ADD_TO_LEXEME(lexeme_buf, c);
    state = next_state;
  }
  return yylex_final_state_to_token_type[state];
}

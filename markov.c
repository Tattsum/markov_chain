#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "emalloc.h"

#define MULTIPLIER 31

enum {
  NPREF = 3, /* プレフィックの語数 */
  NHASH = 4093, /* 状態ハッシュテーブル配列のサイズ*/
  MAXGEN = 10000 /* 生成される単語数の上限*/
};

typedef struct State State;
typedef struct Suffix Suffix;

struct State{ /* プレフィックス + サフィックス */
  char *pref[NPREF]; /* プレフィックスの単語数*/
  Suffix *suf; /* サフィックスリスト*/
  State *next; /*ハッシュテーブル中の次の要素*/
};

struct Suffix { /* サフィックスリスト */
  char *word; /* Suffix */
  Suffix *next; /* サフィックス中の次の要素 */
};

State *statetab[NHASH]; /* 状態のハッシュテーブル */

char NOWWORD[] = "\n"; /* 実際の単語としては絶対に出現しない */

/* hash: NPREF個の文字列からなる配列のハッシュ値を計算 */
unsigned int hash (char *s[NPREF])
{
  unsigned int h;
  unsigned char *p;
  int i;

  h=0;
  for ( i=0; i<NPREF; i++)
    for (p=(unsigned char *) s[i]; *p!='\0'; p++ )
      h = MULTIPLIER * h + *p;

  return h % NHASH;
}

/* lookup: プレフィックスを検索. 指定されればそれを生成*/
/* 見つかるか生成したらポインタを，そうでなければNULLを返す*/
/* 生成作業はstrdmupしないので，あとで文字列が変化してはならない*/
State* lookup (char *prefix[NPREF], int create)
{
  int i, h;
  State *sp;

  h = hash(prefix);
  for (sp=statetab[h]; sp!=NULL; sp=sp->next){
    for(i=0; i<NPREF; i++)
      if(strcmp(prefix[i], sp->pref[i])!=0)
          break;
    if(i==NPREF) /* 見つかった */
      return sp;
  }
  if(create) {
    sp = (State *) emalloc(sizeof(State));
    for(i=0; i<NPREF; i++)
      sp->pref[i] = prefix[i];
    sp->suf=NULL;
    sp->next=statetab[h];
    statetab[h]=sp;
  }
  return sp;
}

/* addsuffix: 状態を追加．あとでサフィックスが変化してはならない */
void addsuffix(State *sp, char *suffix)
{
  Suffix *suf;

  suf=(Suffix *) emalloc(sizeof(Suffix));
  suf->word=suffix;
  suf->next=sp->suf;
  sp->suf=suf;
}

/* add: 単語をサフィックスリストに追加し，プレフィックスを更新 */
void add(char *prefix[NPREF], char *suffix)
{
  State *sp;

  sp=lookup(prefix, 1); /* 見つからなければ生成 */
  addsuffix(sp, suffix);
  /* プレフィックス中の単語を前にずらす */
  memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
  prefix[NPREF-1]=suffix;
}

/* build: 入力を読み，プレフィックステーブルを作成 */
void build(char *prefix[NPREF], FILE *f)
{
  char buf[100], fmt[10];

  /* 書式文字列を作成．ただの%sだとbufがオーバーフローーする可能性がある */
  sprintf(fmt, "%%%lus", sizeof(buf)-1);
  while(fscanf(f, fmt, buf)!=EOF)
    add(prefix, estrdup(buf));
}

/* generate: 1行に1語ずつ出力を生成 */
void generate(int nwords)
{
  State *sp;
  Suffix *suf;
  char *prefix[NPREF], *w;
  int i, nmatch, count=0;

  for(i=0; i<NPREF; i++) /* 初期プレフィックスをリセット */
    prefix[i]=NOWWORD;

  for(i=0; i<nwords; i++){
    sp=lookup(prefix,0);
    nmatch=0;
    for(suf=sp->suf; suf!=NULL; suf=suf->next)
      if(rand() % ++nmatch ==0 ) /* 確率 = 1/nmatch */
        w=suf->word;
    if(strcmp(w, NOWWORD)==0)
      break;
    printf("%s ", w);
    count++;
    memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
    prefix[NPREF-1] = w;
  }
  printf("\n String Length = %d",count);
}

/* markov main: マカロフ連鎖によるランダムテキスト生成プログラム */
int main(void)
{
  int i, nwords=MAXGEN;
  char *prefix[NPREF]; /* 現在の入力プレフィックス */

  for (i=0; i<NPREF; i++) /* 初期プレフィックスをセットアップ */
    prefix[i] = NOWWORD;

  build(prefix,stdin);
  add(prefix, NOWWORD);
  generate(nwords);
  return 0;
}

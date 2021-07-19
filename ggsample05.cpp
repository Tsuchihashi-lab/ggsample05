﻿// ウィンドウ関連の処理
#include "Window.h"

// シェーダー関連の処理
#include "shader.h"

// オブジェクト関連の処理
#include "object.h"

// 変換行列関連の処理
#include "matrix.h"

// Catmull-Rom スプライン
#include "spline.h"

// 四元数
#include "quaternion.h"

// 形状データ
#include "cylinder.h"

// 標準ライブラリ
#include <cmath>

// アニメーションの周期（秒）
const double cycle(5.0);

//
// アプリケーションの実行
//
void app()
{
  // ウィンドウを作成する
  Window window("ggsample05");

  // 背景色を指定する
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // プログラムオブジェクトの作成
  const GLuint program(loadProgram("ggsample05.vert", "pv", "ggsample05.frag", "fc"));

  // uniform 変数のインデックスの検索（見つからなければ -1）
  const GLint mcLoc(glGetUniformLocation(program, "mc"));
  const GLint tLoc(glGetUniformLocation(program, "t"));

  // ビュー変換行列を mv に求める
  GLfloat mv[16];
  lookat(mv, 3.0f, 4.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

  // 頂点配列オブジェクトの作成
  const GLuint vao(createObject(vertices, p0, lines, e));

  // in 変数のインデックスの検索
  GLint p1Loc = glGetAttribLocation(program, "p1");

  // p1 の頂点バッファオブジェクトの作成
  GLuint p1Buf;
  glGenBuffers(1, &p1Buf);
  glBindBuffer(GL_ARRAY_BUFFER, p1Buf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat[3]) * vertices, p1, GL_STATIC_DRAW);

  // 描画に使う頂点配列オブジェクトの指定
  glBindVertexArray(vao);

  // 頂点バッファオブジェクトを in (attribute) 変数 p1 で参照する
  glVertexAttribPointer(p1Loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(p1Loc);

  // 平行移動の経路
  static const float route[][3] =
  {
    { -2.0f, -1.0f, -3.0f },
    {  0.0f, -2.0f, -2.0f },
    { -1.0f, -1.0f, -1.0f },
    { -0.5f, -0.5f, -0.5f },
    {  0.0f,  0.0f,  0.0f },
  };

  // 通過時間 (× cycle)
  static const float transit[] =
  {
    0.0f,
    0.3f,
    0.5f,
    0.7f,
    1.0f,
  };

  // 通過地点の数
  static const int points(sizeof transit / sizeof transit[0]);

  // 経過時間のリセット
  glfwSetTime(0.0);

  // ウィンドウが開いている間繰り返す
  while (window)
  {
    // ウィンドウを消去する
    glClear(GL_COLOR_BUFFER_BIT);

    // シェーダプログラムの使用開始
    glUseProgram(program);

    // 時刻の計測
    const float t(static_cast<float>(fmod(glfwGetTime(), cycle) / cycle));

    // 時刻 t にもとづく回転アニメーション
    GLfloat mr[16];                   // 回転の変換行列
    float q0[4], q1[4], qt[4];
    qmake(q0, 1.0f, 0.0f, 0.0f, 1.0f);
    qmake(q1, 0.0f, 0.0f, 1.0f, 2.0f);
    slerp(qt, q0, q1, t);
    qrot(mr, qt);

    // 時刻 t にもとづく平行移動アニメーション
    float location[3];                // 現在位置
    spline(location, route, transit, points, t);
    GLfloat mt[16];                   // 平行移動の変換行列
    translate(mt, location[0], location[1], location[2]);

    // モデル変換行列を mm に求め，
    // それとビュー変換行列 mv の積をモデルビュー変換行列 mw に求める
    GLfloat mm[16], mw[16];
    multiply(mm, mt, mr);             // モデル変換 mm ← 移動 mt × 回転 mr
    multiply(mw, mv, mm);             // モデルビュー変換 mw ← ビュー変換 mv × モデル変換 mm

    // 透視投影変換行列を mp に求め，
    // それとモデルビュー変換行列 mw の積をクリッピング座標系への変換行列 mc に求める
    GLfloat mp[16], mc[16];
    perspective(mp, 0.5f, window.getAspect(), 1.0f, 15.0f);
    multiply(mc, mp, mw);             // クリッピング座標系への変換行列 mc ← 投影変換 mp × モデルビュー変換 mw

    // uniform 変数 mc に変換行列 mc を設定する
    glUniformMatrix4fv(mcLoc, 1, GL_FALSE, mc);

    // uniform 変数 t に時刻を設定する
    glUniform1f(tLoc, t);

    // 描画に使う頂点配列オブジェクトの指定
    glBindVertexArray(vao);

    // 図形の描画
    glDrawElements(GL_LINES, lines, GL_UNSIGNED_INT, 0);

    // 頂点配列オブジェクトの指定解除
    glBindVertexArray(0);

    // シェーダプログラムの使用終了
    glUseProgram(0);

    // カラーバッファを入れ替えてイベントを取り出す
    window.swapBuffers();
  }
}

#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBitmap>
#include <QMenuBar>
#include <QPen>
#include <QWidget>
#include <QFileDialog>
#include <QFileDialog>
#include <QtSvg/QSvgGenerator>

#include "project.h"

static QString TaskNames[9]= {"Idle","MovingState","AddingState","DeletingState","MovingTransition","AddingTransitionStep1","AddingTranstionStep2","DeletingTransition","Displacing"};

class RenderArea : public QWidget
{
    Q_OBJECT

public:
    enum Tasks { Idle, MovingState, AddingState, DeletingState, MovingTransition, AddingTransitionStep1, AddingTransitionStep2, DeletingTransition, Displacing };
    RenderArea();
    void SetPointers(QWidget *parent,Project *P);
    void Clean();
    void WriteSVG();
    Tasks GetCurrentTask() { return currenttask; };
    void SetCurrentTask(Tasks nt);

public slots:
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);

signals:
    void CreateState(int px,int py);
    void EraseState(QString namestate);
    void CreateTransition(QString transitionorigin,QList<QString> transitionend,int px,int py);
    void EraseTransition(QString nametransition);
    void AddStateMenu(QString namestate);
    void AddTransitionMenu(QString nametransition);
    void CursorError(QString mode,QString curfile);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Project *project;
    QRect sellip;
    QRect tellip;
    float aspectratioellip;
    QPen pen;
    QBrush brush;
    QCursor cur[NUM_CURSORS];
    QFont fnt;
    QPoint disp;
    QPoint stdisp;
    Tasks currenttask;
    Tasks currenttask_beforedisp;
    QString whoismoving;
    QPoint offset;
    QString transitionorigin;
    QList<QString> transitionend;

    void Initialize();

    void DrawState(QPainter *pt,QString namestate);
    void DrawTransition(QPainter *pt,QString nametransition);

    void InitCursor(int curtype);
    QString CursorIntoState(QPoint p);
    QString CursorIntoTransition(QPoint p);

    QSize GetCanvasSize();
};

#endif

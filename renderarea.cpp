#include <QtGui>

#include "renderarea.h"

extern bool DEB;

RenderArea::RenderArea()
{
 project=NULL;

 Initialize();
}

void RenderArea::setPen(const QPen &pen)
{
 this->pen = pen;
 update();
}

void RenderArea::setBrush(const QBrush &brush)
{
 this->brush = brush;
 update();
}

void RenderArea::SetPointers(QWidget *parent,Project *P)
{
 this->setParent(parent);
 project=P;
}

void RenderArea::InitCursor(int curtype)
{
 QString qp;
 QString acs;

 switch (curtype)
 {
  case CUR_ADD_STATE: qp=QString(CUR_BITMAP_ADDST)+QString(CURSOR_SIZE); acs="add_state"; break;
  case CUR_ADD_TRINI: qp=QString(CUR_BITMAP_TRINI)+QString(CURSOR_SIZE); acs="add_transition_1"; break;
  case CUR_ADD_TRFIN: qp=QString(CUR_BITMAP_TRFIN)+QString(CURSOR_SIZE); acs="add_transition_2"; break;
  case CUR_DELETE: qp=QString(CUR_BITMAP_DELETE)+QString(CURSOR_SIZE); acs="delete_st_or_tr"; break;
  default: qp=""; acs="default"; break;
 }

 QBitmap b(qp);
 if (b.isNull())
 {
  emit CursorError(acs,qp);
  cur[curtype] = cur[CUR_DEFAULT];
 }
 else
  cur[curtype] = QCursor(b,b);
}

void RenderArea::Initialize()
{
 offset.setX(0);
 offset.setY(0);

 disp.setX(0);
 disp.setY(0);

 aspectratioellip=ARE;
 sellip.setX(0);
 sellip.setY(0);
 sellip.setWidth(SWIDTH);
 sellip.setHeight(int(SWIDTH/aspectratioellip));

 tellip.setX(0);
 tellip.setY(0);
 tellip.setWidth(TRAD);
 tellip.setHeight(0.62*TRAD);

 fnt.setFamily("TimesRoman");
 fnt.setPointSize(10);

 currenttask=Idle;

 cur[CUR_DEFAULT] = cursor();
 InitCursor(CUR_ADD_STATE);
 InitCursor(CUR_ADD_TRINI);
 InitCursor(CUR_ADD_TRFIN);
 InitCursor(CUR_DELETE);

 setBackgroundRole(QPalette::Base);
 setAutoFillBackground(true);
}

void RenderArea::SetCurrentTask(Tasks nt)
{
 currenttask=nt;
 switch (currenttask)
 {
  case Idle:
  case MovingState:
  case MovingTransition:
  case Displacing: setCursor(cur[CUR_DEFAULT]); break;

  case AddingState: setCursor(cur[CUR_ADD_STATE]); break;
  case DeletingState: setCursor(cur[CUR_DELETE]); break;

  case AddingTransitionStep1: setCursor(cur[CUR_ADD_TRINI]); break;
  case AddingTransitionStep2: setCursor(cur[CUR_ADD_TRFIN]); break;
  case DeletingTransition: setCursor(cur[CUR_DELETE]); break; // The cursor for deleting a state and for deleting a transition is the same

  default:break;
 }
}

QString RenderArea::CursorIntoState(QPoint b)
{
 QString ret=FORBIDDEN_STATE_NAME;

 if (project==NULL)
  return ret;

 QList<QString> k=project->GetStateNameList();
 int i=0;
 while (i<k.size() && ret==FORBIDDEN_STATE_NAME)
 {
  QPoint p=project->GetStatePosition(k[i]);
  if ( (b.x()>p.x()) && (b.x()<p.x()+sellip.width()) && (b.y()>p.y()) && (b.y()<p.y()+sellip.height()) )
   ret=k[i];
  i++;
 }
 return ret;
}

QString RenderArea::CursorIntoTransition(QPoint b)
{
 QString ret=FORBIDDEN_TRANSITION_NAME;

 if (project==NULL)
  return ret;

 QList<QString> k=project->GetTransitionNameList();
 int i=0;
 while (i<k.size() && ret==FORBIDDEN_TRANSITION_NAME)
 {
  QPoint p=project->GetTransitionPosition(k[i]);
  if ( (b.x()>p.x()) && (b.x()<p.x()+sellip.width()) && (b.y()>p.y()) && (b.y()<p.y()+sellip.height()) )
   ret=k[i];
  i++;
 }
 return ret;
}

void RenderArea::paintEvent(QPaintEvent *)
{
 QPainter painter(this);

 painter.setPen(pen);
 painter.setBrush(brush);
 painter.setRenderHint(QPainter::Antialiasing, true);

 QList<QString> k=project->GetStateNameList();
 for (int i=0;i<k.size();i++)
  DrawState(&painter,k[i]);
 k=project->GetTransitionNameList();
 for (int i=0;i<k.size();i++)
  DrawTransition(&painter,k[i]);
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
 Qt::MouseButton b=event->button();
 QPoint pos=event->pos();
 QString ms=CursorIntoState(pos-disp);
 QString mt=CursorIntoTransition(pos-disp);

 if (b==Qt::LeftButton)
 {
  switch (GetCurrentTask())
  {
   case Idle: if (ms!=FORBIDDEN_STATE_NAME)
              {
               SetCurrentTask(MovingState);
               whoismoving=ms;
               offset=pos-project->GetStatePosition(ms);
              }
              if (mt!=FORBIDDEN_TRANSITION_NAME)
              {
               SetCurrentTask(MovingTransition);
               whoismoving=mt;
               offset=pos-project->GetTransitionPosition(mt);
              }
              break;
   case MovingState: break;
   case MovingTransition: break;
   case AddingState: if ((ms==FORBIDDEN_STATE_NAME) && (mt==FORBIDDEN_TRANSITION_NAME))
                     {
                      emit CreateState(pos.x(),pos.y());
                      SetCurrentTask(Idle);
                     }
                     break;
   case DeletingState: if (ms!=FORBIDDEN_STATE_NAME)
                       {
                        emit EraseState(ms);
                        SetCurrentTask(Idle);
                       }
                       break;
   case AddingTransitionStep1: if (ms!=FORBIDDEN_STATE_NAME)
                               {
                                transitionorigin=ms;
                                transitionend.clear();
                                SetCurrentTask(AddingTransitionStep2);
                               }
                               break;
   case AddingTransitionStep2: if ((ms!=FORBIDDEN_STATE_NAME) && (transitionorigin!=FORBIDDEN_STATE_NAME) && (transitionend.indexOf(ms)==-1))
                               {                  
                                transitionend.push_back(ms);
                                SetCurrentTask(AddingTransitionStep2);
                               }
                               break;
   case DeletingTransition: if (mt!=FORBIDDEN_TRANSITION_NAME)
                            {
                             emit EraseTransition(mt);
                             SetCurrentTask(Idle);
                            }
   default: break;
  }
 }

 if ((b==Qt::RightButton) && (GetCurrentTask()==Idle) && (ms!=FORBIDDEN_STATE_NAME))
  emit AddStateMenu(ms);

 if ((b==Qt::RightButton) && (GetCurrentTask()==Idle) && (mt!=FORBIDDEN_TRANSITION_NAME))
  emit AddTransitionMenu(mt);

 if ((b==Qt::RightButton) && (GetCurrentTask()==AddingTransitionStep2) && !transitionend.isEmpty())
 {
  emit CreateTransition(transitionorigin,transitionend,pos.x(),pos.y());
  SetCurrentTask(Idle);
 }

 if ((b==Qt::MiddleButton) && (GetCurrentTask()!=Displacing))
 {
  currenttask_beforedisp=GetCurrentTask();
  SetCurrentTask(Displacing);
  stdisp=pos;
 }

}

void RenderArea::mouseReleaseEvent(QMouseEvent *event)
{
 if (project==NULL)
  return;

 Qt::MouseButton b=event->button();
 if ((b==Qt::LeftButton) && (GetCurrentTask()==MovingState))
 {
  SetCurrentTask(Idle);
  whoismoving=FORBIDDEN_STATE_NAME;
 }
 if ((b==Qt::LeftButton) && (GetCurrentTask()==MovingTransition))
 {
  SetCurrentTask(Idle);
  whoismoving=FORBIDDEN_TRANSITION_NAME;
 }
 if ((b==Qt::MiddleButton) && (GetCurrentTask()==Displacing))
  currenttask=currenttask_beforedisp;
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
 if (project==NULL)
  return;

 QPoint pos=event->pos();
 if (GetCurrentTask()==MovingState)
 {
  project->SetStatePosition(whoismoving,pos-offset);
  update();
 }
 if (GetCurrentTask()==MovingTransition)
 {
  project->SetTransitionPosition(whoismoving,pos-offset);
  update();
 }
 if (GetCurrentTask()==Displacing)
 {
  disp=pos-stdisp;
  update();
 }
}

void RenderArea::Clean()
{
 QPainter pt(this);
 pt.eraseRect( QRect(0, 0, width() - 1, height() - 1) );
}

void RenderArea::DrawState(QPainter *pt,QString namestate)
{
 QString n[2]={
               " "+namestate,
               " "+project->GetHTMLFile(namestate)
              };
 QPoint p=project->GetStatePosition(namestate);

 pt->save();
 pt->translate(p+disp);
 pt->drawEllipse(sellip);

 if (project->GetStateType(namestate)==Initial)
 {
  pt->save();
  QPoint r(5,sellip.height()/2);
  pt->translate(r);
  QRect c(-5,-5,10,10);
  pt->drawEllipse(c);
  pt->restore();
 }

 if (project->GetStateType(namestate)==Terminal)
 {
  int inc=int(0.08*sellip.width()+0.5);
  QPoint q(-inc/2,-inc/2);
  QRect esellip;
  esellip.setWidth(sellip.width()+inc);
  esellip.setHeight(sellip.height()+inc);
  pt->save();
  pt->translate(q);
  /*
  QPoint r(sellip.width(),sellip.height()/2);
  pt->translate(r);
  pt->drawLine( 0, 0,10, 0);
  pt->drawLine(10, 0,10,10);
  pt->drawLine( 2,10,18,10);
  pt->drawLine( 4,13,16,13);
  pt->drawLine( 6,16,14,16);
  */
  pt->drawEllipse(esellip);
  pt->restore();
 }

 QRect trec(0,0,0.9*sellip.width(),0.25*sellip.height());

 QPoint q(0.05*sellip.width(),0.02*sellip.height());


 int cf=fnt.pointSize();
 QRect fontBR;
 float factor;

 for (int i=0;i<2;i++)
 {
  fontBR =pt->fontMetrics().boundingRect(trec,Qt::TextDontClip|Qt::TextWordWrap|Qt::AlignLeft,n[i]);
  factor = trec.height() / fontBR.height();
  if (cf*factor==0)
   fnt.setPointSizeF(1);
  else
  {
   if (cf*factor<0)
    factor=-factor;
   fnt.setPointSizeF(cf*factor);
  }
  pt->save();
  q.setX(0.15*sellip.width());
  float h=0.1+(float(i)+0.5)/4.0;
  q.setY(h*sellip.height());
  pt->translate(q);
  pt->drawText(trec,Qt::AlignLeft,n[i]);
  pt->restore();
 }

 fnt.setPointSize(cf);
 pt->restore();
}

void RenderArea::DrawTransition(QPainter *pt,QString nametrans)
{
 // p0 is the output point of the origin state for this transition
 QPoint  p0=project->GetTransitionInitialPosition(nametrans);

 QVector<QPoint> p1=project->GetTransitionFinalPositions(nametrans);

 if ( (p0==QPoint(0,0)) || (p1[0]==QPoint(0,0)) )
  return;

 QPoint p=project->GetTransitionPosition(nametrans);
 {
  // If the transition has not yet been put in a meaningful position we find out the right one and set it.
  if ((p.x()==0) && (p.y()==0))
  {
   p.setX((p0.x()+p1[0].x()+sellip.width() -tellip.width() )/2);
   p.setY((p0.y()+p1[0].y()+sellip.height()-tellip.height())/2);
   project->SetTransitionPosition(nametrans,p);
  }

  // From now on, p is the upper-left corner of the transition rectangle

  // First, draw the rounded-corner rectangle...
  pt->save();
  pt->translate(p+disp);
  pt->drawRoundedRect(tellip,20.0,15.0,Qt::RelativeSize);
  {
   // Now, write the text with the transition name...
   int cf=fnt.pointSize();
   QRect trec(-0.1*tellip.width(),0.2*tellip.height(),0.9*tellip.width(),0.9*tellip.height());
   QRect fontBR = pt->fontMetrics().boundingRect(trec,Qt::TextDontClip|Qt::TextWordWrap|Qt::AlignLeft,nametrans);
   float factor = 0.8*trec.height() / fontBR.height();
   if (cf*factor==0)
    fnt.setPointSizeF(1);
   else
   {
    if (cf*factor<0)
     factor=-factor;
    fnt.setPointSizeF(cf*factor);
   }
   QPoint q(0.18*tellip.width(),0.2*tellip.height());
   pt->save();
   pt->translate(q);
   pt->drawText(trec,Qt::AlignLeft,nametrans);
   fnt.setPointSize(cf);
   pt->restore();
  }
  pt->restore();

  // Now, draw the transition lines to all its final states.
  for (int i=0;i<p1.size();i++)
  {
   // midA is the output point of the origin state and midB the input point of the transition
   QPoint midA(p0.x()+sellip.width()+disp.x(),p0.y()+sellip.height()/2+disp.y());
   QPoint midB(p.x()+disp.x(),p.y()+tellip.height()/2+disp.y());
   // Now, midC will be the output point of the transition and midC the input point of this destination state
   QPoint midC(p.x()+tellip.width()+disp.x(),p.y()+tellip.height()/2+disp.y());
   QPoint midD(p1[i].x()+disp.x(),p1[i].y()+sellip.height()/2+disp.y());

   int desp=midB.y()-midA.y();
   if (abs(desp)<20)
    pt->drawLine(midA,midB);
   else
   {
    QPoint midA1(midA.x()+10,midA.y());
    QPoint midB1(midB.x()-10,midB.y());
    QPoint midA2(midA1.x(),(midA1.y()+midB1.y())/2);
    QPoint midB2(midB1.x(),(midA1.y()+midB1.y())/2);
    pt->drawLine(midA,midA1);
    pt->drawLine(midB,midB1);
    pt->drawLine(midA1,midA2);
    pt->drawLine(midB1,midB2);
    pt->drawLine(midA2,midB2);
   }

   desp=midD.y()-midC.y();
   if (abs(desp)<20)
    pt->drawLine(midC,midD);
   else
   {
    QPoint midC1(midC.x()+10,midC.y());
    QPoint midD1(midD.x()-10,midD.y());
    QPoint midC2(midC1.x(),(midC1.y()+midD1.y())/2);
    QPoint midD2(midD1.x(),(midC1.y()+midD1.y())/2);
    pt->drawLine(midC,midC1);
    pt->drawLine(midD,midD1);
    pt->drawLine(midC1,midC2);
    pt->drawLine(midD1,midD2);
    pt->drawLine(midC2,midD2);
   }
  }
 }

 // Draw the small half-circle at the input of the transition
 {
  pt->save();
  pt->translate(p+disp);
  QPoint midIT(-5,-5+tellip.height()/2);
  QRect r(midIT,QSize(10,10));
  pt->drawArc(r,-90*16,180*16);
  pt->restore();
 }

 // Draw the small arrow-head at the output of the transition
 {
  pt->save();
  pt->translate(p+disp);
  QPoint mid(tellip.width(),tellip.height()/2);
  QPoint ma=mid+QPoint(-5, 5);
  QPoint mb=mid+QPoint(-5,-5);
  pt->drawLine(ma,mid);
  pt->drawLine(mb,mid);
  pt->restore();
 }

 // Draw the small arrow-head at the output point of the state leading to this transition
 {
  pt->save();
  pt->translate(p0+disp);
  QPoint mid(sellip.width()+8,sellip.height()/2);
  QPoint ma=mid+QPoint(-5, 5);
  QPoint mb=mid+QPoint(-5,-5);
  pt->drawLine(ma,mid);
  pt->drawLine(mb,mid);
  pt->restore();
 }

 // Draw the small arrow-head at the input point of the destination states
 for (int i=0;i<p1.size();i++)
 {
  pt->save();
  pt->translate(p1[i]+disp);
  QPoint mid(0,sellip.height()/2);
  QPoint ma=mid+QPoint(-5, 5);
  QPoint mb=mid+QPoint(-5,-5);
  pt->drawLine(ma,mid);
  pt->drawLine(mb,mid);
  pt->restore();
 }

}

QSize RenderArea::GetCanvasSize()
{
 int maxx=0;
 int maxy=0;
 QPoint q;

 QList<QString> k=project->GetStateNameList();
 for (int i=0;i<k.size();i++)
 {
  q=project->GetStatePosition(k[i]);
  if (q.x()>maxx)
   maxx=q.x();
  if (q.y()>maxy)
   maxy=q.y();
 }

 return(QSize(maxx+SWIDTH+20,maxy+SWIDTH+20));
}

void RenderArea::WriteSVG()
{
 if (project!=NULL)
 {
  QString newPath = QFileDialog::getSaveFileName(this,
         tr("Save SVG"),project->GetPDir(),tr("SVG files (*.svg)"));

  if (newPath.isEmpty())
   return;

  if (!newPath.endsWith(".svg"))
   newPath+=".svg";

  QSvgGenerator generator;
  generator.setFileName(newPath);
  QSize sz=GetCanvasSize();
  generator.setSize(sz);
  generator.setViewBox(QRect(0, 0, sz.width(), sz.height()));
  generator.setTitle("Generated from project file "+project->GetPFile());
  generator.setDescription(project->GetComments());
  QPainter *svgpainter = new QPainter(this);
  svgpainter->begin(&generator);
  svgpainter->setFont(fnt);
  svgpainter->setRenderHint(QPainter::Antialiasing, false);
  svgpainter->setPen(palette().dark().color());
  svgpainter->setBrush(Qt::NoBrush);
  {
   QList<QString> k=project->GetStateNameList();
   for (int i=0;i<k.size();i++)
    DrawState(svgpainter,k[i]);
  }
  {
   QList<QString> k=project->GetTransitionNameList();
   for (int i=0;i<k.size();i++)
    DrawTransition(svgpainter,k[i]);
  }
  svgpainter->end();
 }
}



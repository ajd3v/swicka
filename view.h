#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>

#include "ohlc_provider.h"

class QLabel;
class QSlider;
class QToolButton;

class View;

class GraphicsView : public QGraphicsView {
	Q_OBJECT
	public:
		GraphicsView(View *v) : QGraphicsView(), view(v) { }
	protected:
		void wheelEvent(QWheelEvent *);
		void resizeEvent(QResizeEvent *);
	private:
		View *view;
	signals:
		void resized();
};

class View : public QFrame {
	Q_OBJECT
	public:
		explicit View(const QString &name, QWidget *parent = 0);
		QGraphicsView *view() const;

	public slots:
		void zoomIn(int level = 1);
		void zoomOut(int level = 1);
		void changeDataSource(OHLCProvider* source);
		void redraw();
		void resetView(); // reset to default view

	private slots:
		void setupMatrix();
		void togglePointerMode();
		void toggleOpenGL();
		void toggleAntialiasing();
		void print();
		void candleEntered(QDateTime start);
		void candleLeft();

	private:
		QDateTime viewBegin;
		int viewAtom;
		double zoomLevel;

		OHLCProvider* source;

		QGraphicsScene *scene;

		QGraphicsView *graphicsView;
		QLabel *label;
		QLabel *label2;
		QToolButton *selectModeButton;
		QToolButton *dragModeButton;
		QToolButton *openGlButton;
		QToolButton *antialiasButton;
		QToolButton *printButton;
		QToolButton *resetButton;
		// QSlider *zoomSlider;
};

#endif // VIEW_H
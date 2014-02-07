#include "graph_viewport.h"
#include "ohlc_provider.h"
#include "ohlc_shrinker.h"

#include <assert.h>

GraphViewport::GraphViewport(OHLCProvider* source, float viewMargin) {
	this->source = source;
	this->viewMargin = viewMargin;

	explicitYLimits = false;

	assert(source);

	projection = NULL;
	reset();
}

void GraphViewport::reset() {
	zoomLevel = 0.0;
	viewBegin = source->getMinimum();
	viewEnd = source->getMaximum();
	resetProjection();
}

void GraphViewport::resetProjection() {
	if (projection) {
		delete projection;
	}
	projection = new OHLCShrinker(source, viewBegin, viewEnd, source->getInterval());
	projection->setParent(this);
}

void GraphViewport::zoom(int delta, QDateTime center) {
	double zoomBefore = zoomLevel;
	double zoomAfter = zoomLevel + delta * 0.001;

	qDebug() << "scroll: time=" << center;

	double secondsWidthBase = source->getMaximum().toTime_t() - source->getMinimum().toTime_t();
	double secondsWidthNow = secondsWidthBase / exp(zoomBefore);
	double newSecondsWidth = secondsWidthBase / exp(zoomAfter);

	double newBeginT =
		((double)center.toTime_t())
		-
			(
				((double)center.toTime_t() - (double)viewBegin.toTime_t())
				* (newSecondsWidth) / (secondsWidthNow)
			);

	QDateTime newBegin = QDateTime::fromTime_t(newBeginT);
	if (newBegin < source->getMinimum()) newBegin = source->getMinimum();
	if (newBegin.addSecs(secondsWidthNow) > source->getMaximum()) newBegin = source->getMaximum().addSecs(-secondsWidthNow);

	QDateTime newEnd = newBegin.addSecs(secondsWidthNow);

	if (zoomAfter <= 0) {
		// TODO: or <10 candles shown then
		zoomLevel = 0;
	} else {
		// commit
		viewBegin = newBegin;
		viewEnd = newEnd;
		qDebug() << "new view begin=" << viewBegin << "end=" << viewEnd;
		zoomLevel = zoomAfter;
		resetProjection();
		emit changed();
	}
}

QDateTime GraphViewport::getViewBegin() {
	return viewBegin;
}

QDateTime GraphViewport::getViewEnd() {
	return viewEnd;
}

OHLCProvider* GraphViewport::getSourceProjection() {
	// TODO: shrink to smaller interval...?
	return projection;
}

GraphViewport* GraphViewport::duplicate() {
	GraphViewport *v = new GraphViewport(source, viewMargin);
	v->zoomLevel = zoomLevel;
	v->viewBegin = viewBegin;
	v->viewEnd = viewEnd;
	v->projection = projection;
	return v;
}

GraphRanges GraphViewport::getRanges() {
	GraphRanges ranges;
	ranges.start = viewBegin;
	ranges.end = viewEnd;

	if (explicitYLimits) {
		ranges.priceLow = yLow;
		ranges.priceHigh = yHigh;
	} else {
		OHLC closure;
		if (!OHLC::span(getSourceProjection(), closure)) {
			qDebug() << "cannot construct closure, probably empty. drawing nothing.";
		}

		ranges.priceLow = closure.low;
		ranges.priceHigh = closure.high;
	}

	// TODO: move outside?
	float whitespace = (ranges.priceHigh - ranges.priceLow) * viewMargin;

	// TODO: ensure >=0
	ranges.priceLow -= whitespace / 2;
	ranges.priceHigh += whitespace / 2;

	return ranges;
}

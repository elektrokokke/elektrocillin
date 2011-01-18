#include "piecewiselinearoscillator.h"
#include <cmath>

QPointF PiecewiseLinearOscillator::getNode(int index) const
{
    if (nodes.size()) {
        double phaseOffset = 0.0;
        for (; index < 0; ) {
            index += nodes.size();
            phaseOffset -= 2.0 * M_PI;
        }
        for (; index >= nodes.size(); ) {
            index -= nodes.size();
            phaseOffset += 2.0 * M_PI;
        }
        return QPointF(nodes[index].x() + phaseOffset, nodes[index].y());
    } else {
        return QPointF(index * 2.0 * M_PI, 0.0);
    }
}

int PiecewiseLinearOscillator::size() const
{
    return nodes.size();
}

double PiecewiseLinearOscillator::valueAtPhase(double phase)
{
    Q_ASSERT(phase >= 0.0);
    Q_ASSERT(phase <= 2.0 * M_PI);
    double leftPhase = phase - 0.5 * getPhaseIncrement();
    double rightPhase = phase + 0.5 * getPhaseIncrement();
//    if (leftPhase >= M_PI) {
//        return 1.0;
//    } else if (rightPhase <= M_PI) {
//        return -1.0;
//    } else {
//        return interpolate(QPointF(leftPhase, 1.0), QPointF(rightPhase, -1.0), M_PI);
//    }
    // compute the line segments surrounding the given phase:
    QList<QPointF> intersection;
    int leftNode = 0;
    for (; leftPhase < getNode(leftNode).x(); leftNode--);
    for (; leftPhase > getNode(leftNode+1).x(); leftNode++);
    intersection.append(QPointF(leftPhase, interpolate(getNode(leftNode), getNode(leftNode+1), leftPhase)));
    int rightNode = leftNode+1;
    for (; rightPhase > getNode(rightNode).x(); rightNode++) {
        intersection.append(getNode(rightNode));
    }
    intersection.append(QPointF(rightPhase, interpolate(getNode(rightNode-1), getNode(rightNode), rightPhase)));
    // compute the average y value over these line segments:
    double weightedysum = 0.0;
    double weightsum = 0.0;
    for (int node = 0; node < intersection.size() - 1; node++) {
        double weight = intersection[node+1].x() - intersection[node].x();
        weightedysum += weight * 0.5 * (intersection[node].y() + intersection[node+1].y());
        weightsum += weight;
    }
    if (weightsum) {
        return weightedysum / weightsum;
    } else {
        return 0.0;
    }
}

void PiecewiseLinearOscillator::addNode(const QPointF &node)
{
    Q_ASSERT((node.x() >= 0.0) && (node.x() <= 2.0 * M_PI));
    Q_ASSERT(nodes.isEmpty() || (node.x() >= nodes.last().x()));
    Q_ASSERT((node.y() >= -1.0) && (node.y() <= 1.0));
    nodes.append(node);
}

double PiecewiseLinearOscillator::interpolate(const QPointF &n0, const QPointF &n1, double x)
{
    if (n0.x() == n1.x()) {
        return 0.5 * (n0.y() + n1.y());
    } else {
        return ((x - n0.x()) * n1.y() + (n1.x() - x) * n0.y()) / (n1.x() - n0.x());
    }
}

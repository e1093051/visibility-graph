#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/intersections.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_linear_traits_2.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <CGAL/Arrangement_zone_2.h>
#include <iomanip>
#include <limits>


using namespace std;

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_2 Point;
typedef Kernel::Line_2 Line;
typedef Kernel::Segment_2 Segment;
typedef Kernel::Ray_2 Ray;
typedef CGAL::Arr_linear_traits_2<Kernel> Traits;
typedef CGAL::Arrangement_2<Traits> Arrangement_2;
typedef Traits::X_monotone_curve_2 X_monotone_curve_2;
typedef Arrangement_2::Vertex_handle Vertex_handle;
typedef Arrangement_2::Halfedge_handle Halfedge_handle;

struct DataEntry {
    Kernel::FT slope;
    vector<Point> points;
};

Point get_dual_point_from_line(const Line& l) {
    if (l.is_vertical()) {
        throw std::runtime_error("Vertical line: slope undefined in y = ax - b form.");
    }

    Kernel::FT a = -l.a() / l.b();               
    Kernel::FT b = l.c() / l.b();                  
                                                    
    // std::cout << "a: " << a << std::endl;
    // std::cout << "b: " << b << std::endl;

    return Point(a, b);
}

Point get_dual_point_from_segment(const Segment& seg) {
    const Point& p1 = seg.source();
    const Point& p2 = seg.target();

    Kernel::FT x1 = p1.x(), y1 = p1.y();
    Kernel::FT x2 = p2.x(), y2 = p2.y();

    if (x1 == x2) {
        throw std::runtime_error("Vertical segment: slope undefined in y = ax - b form.");
    }

    Kernel::FT a = (y2 - y1) / (x2 - x1);        // slope
    Kernel::FT b = a * x1 - y1;                 // b = ax - y

    // std::cout << "Segment from " << p1 << " to " << p2 << std::endl;
    // std::cout << "  Dual point: a = " << a << ", b = " << b << std::endl;

    return Point(a, b);
}


// 點 (a, b) 的對偶：y = ax - b → 傳回對應的 Line_2
Line dual_from_point(const Point& pt) {
    Kernel::FT a = pt.x();
    Kernel::FT b = pt.y();

    // 對偶直線 y = ax - b → 選兩個點：(0, -b), (1, a - b)
    return Line(Point(0, -b), Point(1, a - b));
}


Segment segment_from_curve(const X_monotone_curve_2& curve) {
    if (!curve.is_line() && !curve.is_ray()) {
        // Case 1: it's already a segment
        return Segment(curve.source(), curve.target());
    } else {
        // Case 2 & 3: ray or line — use supporting line
        Line line = curve.supporting_line();
        Point source = curve.source();
        Point another = source + (line.to_vector() / 10); // short forward step
        return Segment(source, another);
    }
    throw std::runtime_error("Unknown curve type.");
}


/*
Given several lines, sort their intersections with one specified line by x-coordinate.
*/
pair<array<DataEntry, 2>, vector<DataEntry>> sort_relative_to_lineq(Arrangement_2& arr, const Line& q_star, const Point& ref_point, const vector<Point>& inputPoints) {
    vector<DataEntry> slope_and_points;
    // vector<Point> sorted_points;
    X_monotone_curve_2 q_curve(q_star);
    CGAL::Arr_walk_along_line_point_location<Arrangement_2> pl(arr); // 使用「從左到右掃描的方式」來找位置
    vector<CGAL::Object> zone_result;
    CGAL::zone(arr, q_curve, back_inserter(zone_result), pl);

    array<DataEntry, 2> vertical_group;
    
    vertical_group[0].slope = numeric_limits<Kernel::FT>::infinity();  // 表示朝上
    vertical_group[1].slope = -numeric_limits<Kernel::FT>::infinity();   // 表示朝下
    for (const Point& point : inputPoints) {
        if (point.x() == ref_point.x()) {
            if (point.y() > ref_point.y()) {
                vertical_group[0].points.push_back(point); // 上方
            }
            else if (point.y() < ref_point.y()) {
                vertical_group[1].points.push_back(point); // 下方
            }
        }
    }
    // 上方：依 y 從小到大排序
    sort(vertical_group[0].points.begin(), vertical_group[0].points.end(),
    [](const Point& a, const Point& b) {
        return a.y() < b.y();
    });

    // 下方：依 y 從大到小排序
    sort(vertical_group[1].points.begin(), vertical_group[1].points.end(),
    [](const Point& a, const Point& b) {
        return a.y() > b.y();
    });


    for (const CGAL::Object& obj : zone_result) {
        Vertex_handle v;
        if (CGAL::assign(v, obj)) {
            if (v->is_at_open_boundary()) continue;
            // sorted_points.push_back(v->point());
            // Iterate over incident halfedges (each line incident to the vertex)
            Arrangement_2::Halfedge_around_vertex_const_circulator circ = v->incident_halfedges();
            Arrangement_2::Halfedge_around_vertex_const_circulator done = circ;
            // std::cout << "Intersection at: " << v->point() << " belongs to lines:\n";
            // 先計算有幾條 halfedge
            DataEntry entry;
            int total = 0;
            do {
                ++total;
                ++circ;
            } while (circ != done);
            circ = v->incident_halfedges(); // reset
            int limit = total / 2;
            int count = 0;
            entry.slope = v->point().x();
            // cout << "slope: " << v->point().x() << endl;
            do {
                const auto& curve = circ->curve(); // The underlying X_monotone_curve_2
                if (curve.supporting_line() == q_star) {
                    --circ;
                    ++count;
                    continue; // ✅ 跳過 q_star 自己的部分
                }
                Point dp = get_dual_point_from_segment(segment_from_curve(curve));
                entry.points.push_back(dp);
                // cout << dp << endl;
                --circ;
                ++count;
            } while (count < limit);
            std::sort(entry.points.begin(), entry.points.end(),
                [&](const Point& a, const Point& b) {
                    Kernel::FT dx1 = a.x() - ref_point.x();
                    Kernel::FT dy1 = a.y() - ref_point.y();
                    Kernel::FT dx2 = b.x() - ref_point.x();
                    Kernel::FT dy2 = b.y() - ref_point.y();
                    return (dx1*dx1 + dy1*dy1) < (dx2*dx2 + dy2*dy2);
                }
            );
            slope_and_points.push_back(entry);
        }
    }
    /*
    for (const auto& entry : slope_and_points) {
        std::cout << "Slope: " << entry.slope << std::endl;
        std::cout << "Points:" << std::endl;
        for (const auto& pt : entry.points) {
            std::cout << "  (" << pt.x() << ", " << pt.y() << ")" << std::endl;
        }
        std::cout << "----------------------" << std::endl;
    } 
    */   
    return {vertical_group, slope_and_points};
}

/*
Given order of slope, sort by angle.
Start from pi/2, sweep counter clockwise.
p is the reference point
*/
vector<Point> sort_by_angle(vector<DataEntry> slope_and_points, array<DataEntry, 2> vertical_group, Point reference) {
    // for each quardrant, go through the complete sorted slope
    std::vector<Point> sorted_points;
    // right right（右） to reference point
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            if (p.x() > reference.x() && p.y() == reference.y()) {
                sorted_points.push_back(p);
            }
        }
    }



    // insert points from the first quadrant == x - reference.x > 0, y - reference.y > 0
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            // std::cout << "  (" << p.x() << ", " << p.y() << ")\n";
            if (p.x() > reference.x() && p.y() > reference.y()) {
                sorted_points.push_back(p);
            }
        }
    }
    // right above reference point
    for (const auto& p : vertical_group[0].points) {
        sorted_points.push_back(p);
    }
    
    // insert points from the second quadrant == x - reference.x < 0, y - reference.y > 0
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            // std::cout << "  (" << p.x() << ", " << p.y() << ")\n";
            if (reference.x() > p.x() && p.y() > reference.y()) {
                sorted_points.push_back(p);
            }
        }
    }
    // right left to reference point
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            if (reference.x() > p.x() && p.y() == reference.y()) {
                sorted_points.push_back(p);
            }
        }
    }
    // insert points from the third quadrant == x - reference.x < 0, y - reference.y < 0
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            // std::cout << "  (" << p.x() << ", " << p.y() << ")\n";
            if (reference.x() > p.x() && reference.y() > p.y()) {
                sorted_points.push_back(p);
            }
        }
    }
    // right below to reference point
    for (const auto& p : vertical_group[1].points) {
        sorted_points.push_back(p);
    }
    // insert points from the fourth quadrant == x - reference.x > 0, y - reference.y < 0
    for (const auto& entry : slope_and_points) {
        // std::cout << "Slope: " << entry.slope << "\nPoints:\n";
        for (const auto& p : entry.points) {
            // std::cout << "  (" << p.x() << ", " << p.y() << ")\n";
            if (p.x() > reference.x() && reference.y() > p.y()) {
                sorted_points.push_back(p);
            }
        }
    }
    return sorted_points;
}




void point_line_point_check(Point p) {
    Line l = dual_from_point(p);
    Point p2 = get_dual_point_from_line(l);
    if (p == p2) {
        cout << "safe" << endl;
    }
    else {
        cout << "not safe" << endl;
    }
}


vector<Point> process_points_and_sort_by_angle(const vector<Point>& inputPoints, const Point& ref_point) {
    Arrangement_2 arr;

    for (const Point& pt : inputPoints) {
        Line l = dual_from_point(pt);
        insert(arr, X_monotone_curve_2(l));
    }

    Line q_star = dual_from_point(ref_point);

    auto [vertical_group, rawResult] = sort_relative_to_lineq(arr, q_star, ref_point, inputPoints);
    std::vector<Point> sortedPoints = sort_by_angle(rawResult, vertical_group, ref_point);

    return sortedPoints;
}


int main() {
    int n;
    cin >> n;

    vector<Point> inputPoints;
    for (int i = 0; i < n; i++) {
        double x, y;
        cin >> x >> y;
        inputPoints.emplace_back(x, y);
    }

    Arrangement_2 arr;
    for (const Point& pt : inputPoints) {
        Line l = dual_from_point(pt);
        insert(arr, X_monotone_curve_2(l));
    }

    std::cout << "INIT_DONE" << std::endl;
    std::cout.flush();


    string cmd;
    while (cin >> cmd) {
        if (cmd == "QUERY") {
            double rx, ry;
            cin >> rx >> ry;
            Point ref(rx, ry);
            Line q_star = dual_from_point(ref);
            auto [vertical_group, result] = sort_relative_to_lineq(arr, q_star, ref, inputPoints);
            // cout << "result lengh " << result.size() << endl;
            vector<Point> sorted_points = sort_by_angle(result, vertical_group, ref);
            std::cout << std::fixed << std::setprecision(8)
                << CGAL::to_double(ref.x()) << " " 
                << CGAL::to_double(ref.y()) << std::endl;
            for (const auto& pt : sorted_points) {
                std::cout << std::fixed << std::setprecision(8)
                          << CGAL::to_double(pt.x()) << " " 
                          << CGAL::to_double(pt.y()) << std::endl;
            }
            std::cout << "END_OUTPUT" << std::endl;
        } else if (cmd == "END") {
            break;
        }
    }

    return 0;
}



/*
test case:
10
-10 70
-6 66
-8 68
20 40
40 20
0 60
-15 75
-40 100
100 -40
160 -100
QUERY
0 60
*/
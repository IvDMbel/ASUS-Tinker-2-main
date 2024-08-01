#pragma once
#define _USE_MATH_DEFINES
 
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <variant>
 
namespace svg {

 struct Point {
        Point() = default;
        Point(double x, double y) : x(x)
            , y(y) {}
        double x = 0.0;
        double y = 0.0;
    };
 
struct Rgb {

    Rgb() = default;
    Rgb(int red, int green, int blue);
    
    int red_ = 0;
    int green_ = 0;
    int blue_ = 0;
};
inline void PrintColor(std::ostream& out, Rgb& rgb);
 
struct Rgba {

    Rgba() = default;
    Rgba(int red, int green, int blue, double opacity);
    
    int red_ = 0;
    int green_ = 0;
    int blue_ = 0;
    double opacity_ = 1.0;
};  
inline void PrintColor(std::ostream& out, Rgba& rgba);
 
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};  
    
inline void PrintColor(std::ostream& out, std::monostate);
inline void PrintColor(std::ostream& out, std::string& color);
std::ostream& operator<<(std::ostream& out, const Color& color); 
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineCap stroke_line_cap) {
    using namespace std::literals;
    std::string_view sv;
    switch (stroke_line_cap) {
    case svg::StrokeLineCap::BUTT:  sv = "butt"sv;
        break;
    case svg::StrokeLineCap::ROUND: sv = "round"sv;
        break;
    case svg::StrokeLineCap::SQUARE: sv = "square"sv;
        break;
    default:
        break;
    }
    return out << sv;
}
 
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
inline std::ostream &operator<<(std::ostream &out, StrokeLineJoin stroke_line_join) {
    using namespace std::literals;
    
    std::string_view sv;
    switch (stroke_line_join)
    {
    case svg::StrokeLineJoin::ARCS: sv = "arcs"sv;
        break;

    case svg::StrokeLineJoin::BEVEL: sv = "bevel"sv;
        break;
    case svg::StrokeLineJoin::MITER: sv = "miter"sv;
        break;
    case svg::StrokeLineJoin::MITER_CLIP: sv = "miter-clip"sv;
        break;
    case svg::StrokeLineJoin::ROUND: sv = "round"sv;
        break;
    default:
        break;
    }
    return out << sv;
}
    
template<typename Owner>
class PathProps {
public:
    Owner& SetFillColor(const Color& color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
 
    Owner& SetStokeColor(const Color& color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
 
    Owner& SetStokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
 
    Owner& SetStokeLinecap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
 
    Owner& SetStokeLinejoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    } 
 
protected:
    ~PathProps() = default;
 
    void RenderAttrs(std::ostream &out) const {
        using namespace std::literals;
 
        if (fill_color_ != std::nullopt) {
            out << "fill=\""sv << *fill_color_ << "\" "sv;
        }
        if (stroke_color_ != std::nullopt) {
            out << "stroke=\""sv << *stroke_color_ << "\" "sv;
        }
        if (stroke_width_ != std::nullopt) {
            out << "stroke-width=\""sv << *stroke_width_ << "\" "sv;
        }
        if (stroke_line_cap_ != std::nullopt) {
            out << "stroke-linecap=\""sv << *stroke_line_cap_ << "\" "sv;
        }
        if (stroke_line_join_ != std::nullopt) {
            out << "stroke-linejoin=\""sv << *stroke_line_join_ << "\" "sv;
        }
    }
    
private: 
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
    
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }
};
    

 
struct RenderContext {
    RenderContext(std::ostream& out);
    RenderContext(std::ostream& out, int indent_step, int indent = 0) : out_(out)
                                                                      , indent_step_(indent_step)
                                                                      , indent_(indent) {}
    RenderContext Indented() const;
    void RenderIndent() const;
 
    std::ostream& out_;
    int indent_step_ = 0;
    int indent_ = 0;
};
 
class Object {
public:
    void Render(const RenderContext& context) const;
    virtual ~Object() = default;
 
private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
 
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
 
private:
    Point center_;
    double radius_ = 1.0;
    
    void RenderObject(const RenderContext& context) const override;
};
 
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);
    
private:
    std::vector<Point> points_;
    void RenderObject(const RenderContext& context) const override;
};
 
class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);
 
private:
    Point position_;
    Point offset_;
    std::string font_family_;
    std::string font_weight_;
    uint32_t font_size_ = 1;
    std::string data_;
     
    static std::string DeleteSpaces(const std::string& str);
    static std::string UniqSymbols(const std::string& str);
    
    void RenderObject(const RenderContext& context) const override;
};
 
class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;
 
    template<typename Obj>
    void Add(Obj obj);
 
    virtual void AddPtr(std::unique_ptr<Object> &&) = 0;
 
protected:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
template<typename Obj>
void ObjectContainer::Add(Obj obj) {
    objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}
    
class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};
    
class Document : public ObjectContainer {
public: 
    
    void AddPtr(std::unique_ptr<Object>&& obj) override;
     
    void Render(std::ostream& out) const;
    
};
    
}//end namespace svg

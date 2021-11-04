#include "graphic/text.h"

ygl::graphic::text::text() :
	_data(0),
	_font(nullptr),
	_filename(std::string()),
	_measure(0),
	_buffer(std::string()),
	_message(std::string()),
	_align(ygl::graphic::align::ALIGN_LEFT),
	_weight(ygl::graphic::weight::WEIGHT_NORMAL),
	_visible(true),
	_wireframe(false),
	_flip(ygl::graphic::flip::FLIP_NONE),
	_color(ygl::math::colord().WHITE()),
	_opacity(100),
	_orientation(ygl::graphic::orientation::ORIENTATION_GLOBAL),
	_pivot(ygl::math::pointd(0.5)),
	_position(ygl::math::pointd()),
	_scale(ygl::math::sized(1)),
	_rotate(ygl::math::rotated()),
	_size(ygl::math::sized()),
	_offset(ygl::math::surfaced(0.0, 0.0, 1.0, 1.0)),
	_vertex(ygl::math::vertexd()),
	_map(ygl::math::uvmapd(1.0)),
	_running(false),
	_animation(std::string()),
	_sensible(std::map<std::string, ygl::math::surfaced>()),
	_tweening(std::map<std::string, ygl::animation::tween*>())
{
}

ygl::graphic::text::text(const std::string& filename, std::size_t measure) :
	_data(0),
	_font(nullptr),
	_filename(std::string()),
	_measure(0),
	_buffer(std::string()),
	_message(std::string()),
	_align(ygl::graphic::align::ALIGN_LEFT),
	_weight(ygl::graphic::weight::WEIGHT_NORMAL),
	_visible(true),
	_wireframe(false),
	_flip(ygl::graphic::flip::FLIP_NONE),
	_color(ygl::math::colord().WHITE()),
	_opacity(100),
	_orientation(ygl::graphic::orientation::ORIENTATION_GLOBAL),
	_pivot(ygl::math::pointd(0.5)),
	_position(ygl::math::pointd()),
	_scale(ygl::math::sized(1)),
	_rotate(ygl::math::rotated()),
	_size(ygl::math::sized()),
	_vertex(ygl::math::vertexd()),
	_map(ygl::math::uvmapd(1.0)),
	_running(false),
	_animation(std::string()),
	_sensible(std::map<std::string, ygl::math::surfaced>()),
	_tweening(std::map<std::string, ygl::animation::tween*>())
{
	this->initialize(filename, measure);
}

ygl::graphic::text::~text()
{
	this->destroy();
}

bool ygl::graphic::text::initialize(const std::string& filename, std::size_t measure)
{
	this->_filename = filename;
	this->_font = TTF_OpenFont(filename.c_str(), measure);

	this->_color.fill(255, 255, 255);
	this->_opacity = 100;
	this->_scale.fill(1, 1, 1);

	if (this->_font == nullptr)
	{
		std::cerr << "ERROR: Failing initializing ttf " << filename << " -- " << TTF_GetError() << std::endl;
		return false;
	}

	this->_measure = measure;

	return true;
}

void ygl::graphic::text::destroy()
{
	glDeleteTextures(1, &this->_data);
	TTF_CloseFont(this->_font);

	this->_font = nullptr;
	this->_sensible.clear();
	this->_tweening.clear();
}

ygl::animation::tween* ygl::graphic::text::tweening(const std::string& animation)
{
	auto it = this->_tweening.find(animation);
	if (it == this->_tweening.end())
	{
		std::cerr << "Text animation '" << animation << "' not found, failed" << std::endl;
		exit(0);
	}
	return this->_tweening[animation];
}

bool ygl::graphic::text::animation(const std::string& animation, std::size_t timeline)
{
	auto it = this->_tweening.find(animation);

	if (it != this->_tweening.end())
	{
		return false;
	}

	if (this->_tweening.size() == 0)
	{
		this->_animation = animation;
	}

	this->_tweening.insert(std::pair<std::string, ygl::animation::tween*>(animation, new ygl::animation::tween(animation, timeline)));

	return true;
}

bool ygl::graphic::text::animation(const std::string& animation, std::size_t timeline, std::size_t fps)
{
	auto it = this->_tweening.find(animation);

	if (it != this->_tweening.end())
	{
		return false;
	}

	if (this->_tweening.size() == 0)
	{
		this->_animation = animation;
	}

	this->_tweening.insert(std::pair<std::string, ygl::animation::tween*>(animation, new ygl::animation::tween(animation, timeline, fps)));

	return true;
}

void ygl::graphic::text::animation(const std::string& animation)
{
	auto it = this->_tweening.find(animation);

	if (it == this->_tweening.end())
	{
		std::cerr << "Image animation '" << animation << "' not found, failed" << std::endl;

		exit(0);
	}

	this->_animation = animation;
}

bool ygl::graphic::text::sensible(const std::string& name, const ygl::math::surfaced& sensible)
{
	auto it = this->_sensible.find(name);

	if (it != this->_sensible.end())
	{
		return false;
	}

	this->_sensible.insert(std::pair<std::string, ygl::math::surfaced>(name, sensible));

	glGenTextures(1, &this->_data);

	return true;
}

const ygl::math::surfaced& ygl::graphic::text::sensible(const std::string& name)
{
	auto it = this->_sensible.find(name);

	if (it == this->_sensible.end())
	{
		std::cerr << "Layer sensible not found" << std::endl;
		exit(1);
	}

	return this->_sensible[name];
}

void ygl::graphic::text::draw()
{
	if (this->_visible && this->_opacity > 0 && !this->_message.empty())
	{
		this->resolve();

		glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, this->_data);

		glTranslated(this->_position.x(), this->_position.y(), ygl::math::invert(this->_position.z()));

		glRotated(this->_rotate.h(), 1.0, 0.0, 0.0);
		glRotated(this->_rotate.p(), 0.0, 1.0, 0.0);
		glRotated(this->_rotate.b(), 0.0, 0.0, 1.0);

		glScaled(this->_scale.w(), this->_scale.h(), this->_scale.l());

		glColor4d(
			ygl::math::normal_channel(this->_color.r()),
			ygl::math::normal_channel(this->_color.g()),
			ygl::math::normal_channel(this->_color.b()),
			ygl::math::normal_opacity(this->_opacity)
		);

		switch (this->_flip)
		{
			case ygl::graphic::flip::FLIP_NONE:
			{
				glBegin(GL_QUADS);

				glTexCoord2d(this->_offset.x(), this->_offset.y()); glVertex3d(this->_vertex.a().x(), this->_vertex.a().y(), this->_vertex.a().z());
				glTexCoord2d(this->_offset.w(), this->_offset.y()); glVertex3d(this->_vertex.b().x(), this->_vertex.b().y(), this->_vertex.b().z());
				glTexCoord2d(this->_offset.w(), this->_offset.h()); glVertex3d(this->_vertex.c().x(), this->_vertex.c().y(), this->_vertex.c().z());
				glTexCoord2d(this->_offset.x(), this->_offset.h()); glVertex3d(this->_vertex.d().x(), this->_vertex.d().y(), this->_vertex.d().z());

				glEnd();

				break;
			}
			case ygl::graphic::flip::FLIP_HORIZONTAL:
			{
				glBegin(GL_QUADS);

				glTexCoord2d(this->_offset.x(), this->_offset.y()); glVertex3d(this->_vertex.b().x(), this->_vertex.b().y(), this->_vertex.b().z());
				glTexCoord2d(this->_offset.w(), this->_offset.y()); glVertex3d(this->_vertex.a().x(), this->_vertex.a().y(), this->_vertex.a().z());
				glTexCoord2d(this->_offset.w(), this->_offset.h()); glVertex3d(this->_vertex.d().x(), this->_vertex.d().y(), this->_vertex.d().z());
				glTexCoord2d(this->_offset.x(), this->_offset.h()); glVertex3d(this->_vertex.c().x(), this->_vertex.c().y(), this->_vertex.c().z());

				glEnd();

				break;
			}
			case ygl::graphic::flip::FLIP_VERTICAL:
			{
				glBegin(GL_QUADS);

				glTexCoord2d(this->_offset.x(), this->_offset.y()); glVertex3d(this->_vertex.d().x(), this->_vertex.d().y(), this->_vertex.d().z());
				glTexCoord2d(this->_offset.w(), this->_offset.y()); glVertex3d(this->_vertex.c().x(), this->_vertex.c().y(), this->_vertex.c().z());
				glTexCoord2d(this->_offset.w(), this->_offset.h()); glVertex3d(this->_vertex.b().x(), this->_vertex.b().y(), this->_vertex.b().z());
				glTexCoord2d(this->_offset.x(), this->_offset.h()); glVertex3d(this->_vertex.a().x(), this->_vertex.a().y(), this->_vertex.a().z());

				glEnd();

				break;
			}
			case ygl::graphic::flip::FLIP_ALL:
			{
				glBegin(GL_QUADS);

				glTexCoord2d(this->_offset.x(), this->_offset.y()); glVertex3d(this->_vertex.c().x(), this->_vertex.c().y(), this->_vertex.c().z());
				glTexCoord2d(this->_offset.w(), this->_offset.y()); glVertex3d(this->_vertex.d().x(), this->_vertex.d().y(), this->_vertex.d().z());
				glTexCoord2d(this->_offset.w(), this->_offset.h()); glVertex3d(this->_vertex.a().x(), this->_vertex.a().y(), this->_vertex.a().z());
				glTexCoord2d(this->_offset.x(), this->_offset.h()); glVertex3d(this->_vertex.b().x(), this->_vertex.b().y(), this->_vertex.b().z());

				glEnd();

				break;
			}
		}

		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_MATERIAL);

		if (this->_wireframe)
		{
			glEnable(GL_LINE_STIPPLE);

			glTranslated(this->_position.x(), this->_position.y(), this->_position.z());

			glRotated(this->_rotate.h(), 1.0, 0.0, 0.0);
			glRotated(this->_rotate.p(), 0.0, 1.0, 0.0);
			glRotated(this->_rotate.b(), 0.0, 0.0, 1.0);

			glScaled(this->_scale.w(), this->_scale.h(), this->_scale.l());

			glColor4d(0.0, 1.0, 0.0, 1.0);

			glLineWidth(2.0);
			glBegin(GL_LINE_LOOP);
			glVertex3d(this->_vertex.a().x(), this->_vertex.a().y(), this->_vertex.a().z());
			glVertex3d(this->_vertex.b().x(), this->_vertex.b().y(), this->_vertex.b().z());
			glVertex3d(this->_vertex.c().x(), this->_vertex.c().y(), this->_vertex.c().z());
			glVertex3d(this->_vertex.d().x(), this->_vertex.d().y(), this->_vertex.d().z());
			glEnd();

			glColor4d(0.0, 0.0, 1.0, 1.0);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glVertex3d(-10.0, 0.0, 0.0);
			glVertex3d( 10.0, 0.0, 0.0);
			glEnd();

			glColor4d(1.0, 0.0, 0.0, 1.0);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			glVertex3d(0.0, -10.0, 0.0);
			glVertex3d(0.0,  10.0, 0.0);
			glEnd();

			glDisable(GL_LINE_STIPPLE);
		}

		glPopMatrix();
	}
}

void ygl::graphic::text::draw(const std::string& message, const ygl::math::pointd& position, const ygl::math::rotated& rotate, const ygl::math::sized& scale, const ygl::math::colord& color, const ygl::graphic::align& align)
{
	if (this->_message != message && !message.empty())
	{
		this->message(message);
	}

	if (this->_position != position && position != ygl::math::pointd().is_null())
	{
		this->position(position);
	}

	if (this->_rotate != rotate && rotate != ygl::math::rotated().is_null())
	{
		this->rotate(rotate);
	}

	if (this->_scale != scale && scale != ygl::math::sized().is_null())
	{
		this->scale(scale);
	}

	if (this->_color != color && color != ygl::math::colord().is_null())
	{
		this->color(color);
	}

	if (this->_align != align && align != ygl::graphic::align::ALIGN_NULL)
	{
		this->align(align);
	}

	this->draw();
}

void ygl::graphic::text::running()
{
	if (!this->_running)
	{
		return;
	}

	if (this->_tweening.empty())
	{
		return;
	}

	if (!this->_tweening[this->_animation]->has_tweening())
	{
		return;
	}

	if (this->_tweening[this->_animation]->has_visible())
	{
		this->visible(this->_tweening[this->_animation]->visible());
	}

	if (this->_tweening[this->_animation]->has_wireframe())
	{
		this->wireframe(this->_tweening[this->_animation]->wireframe());
	}

	if (this->_tweening[this->_animation]->has_flip())
	{
		this->flip(this->_tweening[this->_animation]->flip());
	}

	if (this->_tweening[this->_animation]->has_color())
	{
		this->color(this->_tweening[this->_animation]->color());
	}

	if (this->_tweening[this->_animation]->has_opacity())
	{
		this->opacity(this->_tweening[this->_animation]->opacity());
	}

	if (this->_tweening[this->_animation]->has_orientation())
	{
		this->orientation(this->_tweening[this->_animation]->orientation());
	}

	if (this->_tweening[this->_animation]->has_pivot())
	{
		this->pivot(this->_tweening[this->_animation]->pivot());
	}

	if (this->_tweening[this->_animation]->has_position())
	{
		this->position(this->_tweening[this->_animation]->position());
	}

	if (this->_tweening[this->_animation]->has_scale())
	{
		this->scale(this->_tweening[this->_animation]->scale());
	}

	if (this->_tweening[this->_animation]->has_rotate())
	{
		this->rotate(this->_tweening[this->_animation]->rotate());
	}

	this->_tweening[this->_animation]->running();
}

void ygl::graphic::text::start()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			this->_tweening[this->_animation]->start();
			this->_running = true;
		}
	}
}

void ygl::graphic::text::pause()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			this->_tweening[this->_animation]->pause();
			this->_running = false;
		}
	}
}

void ygl::graphic::text::restart()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			this->_tweening[this->_animation]->restart();
			this->_running = true;
		}
	}
}

void ygl::graphic::text::reset()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			this->_tweening[this->_animation]->reset();
			this->_running = false;
		}
	}
}

bool ygl::graphic::text::started()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			return this->_tweening[this->_animation]->started() && this->_running;
		}
	}

	return false;
}

bool ygl::graphic::text::border()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			return this->_tweening[this->_animation]->border();
		}
	}

	return false;
}

bool ygl::graphic::text::onset()
{
	if (!this->_animation.empty())
	{
		if (this->_tweening[this->_animation]->has_tweening())
		{
			return this->_tweening[this->_animation]->onset();
		}
	}

	return false;
}

void ygl::graphic::text::message(const std::string& message)
{
	this->_message = message;
}

void ygl::graphic::text::align(const ygl::graphic::align& align)
{
	this->_align = align;
}

void ygl::graphic::text::weight(const ygl::graphic::weight& weight)
{
	this->_weight = weight;

	switch (this->_weight)
	{
		case ygl::graphic::weight::WEIGHT_BOLD:
		{
			TTF_SetFontStyle(this->_font, TTF_STYLE_BOLD);
			break;
		}
		case ygl::graphic::weight::WEIGHT_ITALIC:
		{
			TTF_SetFontStyle(this->_font, TTF_STYLE_ITALIC);
			break;
		}
		case ygl::graphic::weight::WEIGHT_UNDERLINE:
		{
			TTF_SetFontStyle(this->_font, TTF_STYLE_UNDERLINE);
			break;
		}
		case ygl::graphic::weight::WEIGHT_STRIKETHROUGH:
		{
			TTF_SetFontStyle(this->_font, TTF_STYLE_STRIKETHROUGH);
			break;
		}
		case ygl::graphic::weight::WEIGHT_NORMAL:
		{
			TTF_SetFontStyle(this->_font, TTF_STYLE_NORMAL);
			break;
		}
	}
}

void ygl::graphic::text::visible(bool visible)
{
	this->_visible = visible;
}

void ygl::graphic::text::wireframe(bool wireframe)
{
	this->_wireframe = wireframe;
}

void ygl::graphic::text::flip(const ygl::graphic::flip& flip)
{
	this->_flip = flip;
}

void ygl::graphic::text::color(const ygl::math::colord& color)
{
	this->_color = color;
}

void ygl::graphic::text::color(double r, double g, double b)
{
	this->_color.fill(r, g, b);
}

void ygl::graphic::text::color_r(double r)
{
	this->_color.r(r);
}

void ygl::graphic::text::color_g(double g)
{
	this->_color.g(g);
}

void ygl::graphic::text::color_b(double b)
{
	this->_color.b(b);
}

void ygl::graphic::text::opacity(double opacity)
{
	this->_opacity = ygl::math::clamp(opacity, 0.0, 100.0);
}

void ygl::graphic::text::orientation(const ygl::graphic::orientation& orientation)
{
	this->_orientation = orientation;
}

void ygl::graphic::text::pivot(const ygl::math::pointd& pivot)
{
	this->_pivot = pivot;
}

void ygl::graphic::text::pivot(double x, double y, double z)
{
	this->_pivot.fill(x, y, z);
}

void ygl::graphic::text::pivot_x(double x)
{
	this->_pivot.x(x);
}

void ygl::graphic::text::pivot_y(double y)
{
	this->_pivot.y(y);
}

void ygl::graphic::text::pivot_z(double z)
{
	this->_pivot.z(z);
}

void ygl::graphic::text::position(const ygl::math::pointd& position)
{
	this->_position = position;
}

void ygl::graphic::text::position(double x, double y, double z)
{
	this->_position.fill(x, y, z);
}

void ygl::graphic::text::position_x(double x)
{
	this->_position.x(x);
}

void ygl::graphic::text::position_y(double y)
{
	this->_position.y(y);
}

void ygl::graphic::text::position_z(double z)
{
	this->_position.z(z);
}

void ygl::graphic::text::scale(const ygl::math::sized& scale)
{
	this->_scale = scale;
}

void ygl::graphic::text::scale(double w, double h, double l)
{
	this->_scale.fill(w, h, l);
}

void ygl::graphic::text::scale_w(double w)
{
	this->_scale.w(w);
}

void ygl::graphic::text::scale_h(double h)
{
	this->_scale.h(h);
}

void ygl::graphic::text::scale_l(double l)
{
	this->_scale.l(l);
}

void ygl::graphic::text::rotate(const ygl::math::rotated& rotate)
{
	this->_rotate = rotate;
}

void ygl::graphic::text::rotate(double h, double p, double b)
{
	this->_rotate.fill(h, p, b);
}

void ygl::graphic::text::rotate_h(double h)
{
	this->_rotate.h(h);
}

void ygl::graphic::text::rotate_p(double p)
{
	this->_rotate.p(p);
}

void ygl::graphic::text::rotate_b(double b)
{
	this->_rotate.b(b);
}

std::size_t ygl::graphic::text::measure() const
{
	return this->_measure;
}

const std::string& ygl::graphic::text::message() const
{
	return this->_message;
}

const ygl::graphic::align& ygl::graphic::text::align() const
{
	return this->_align;
}

const ygl::graphic::weight& ygl::graphic::text::weight() const
{
	return this->_weight;
}

bool ygl::graphic::text::visible() const
{
	return this->_visible;
}

bool ygl::graphic::text::wireframe() const
{
	return this->_wireframe;
}

const ygl::graphic::flip& ygl::graphic::text::flip() const
{
	return this->_flip;
}

const ygl::math::colord& ygl::graphic::text::color() const
{
	return this->_color;
}

double ygl::graphic::text::opacity() const
{
	return this->_opacity;
}

const ygl::graphic::orientation& ygl::graphic::text::orientation() const
{
	return this->_orientation;
}

const ygl::math::pointd& ygl::graphic::text::pivot() const
{
	return this->_pivot;
}

const ygl::math::pointd& ygl::graphic::text::position() const
{
	return this->_position;
}

const ygl::math::sized& ygl::graphic::text::scale() const
{
	return this->_scale;
}

const ygl::math::rotated& ygl::graphic::text::rotate() const
{
	return this->_rotate;
}

const ygl::math::sized& ygl::graphic::text::size() const
{
	return this->_size;
}

void ygl::graphic::text::resolve()
{
	if (this->_message != this->_buffer && !this->_message.empty())
	{
		this->_buffer = this->_message;

		SDL_Surface* image = nullptr;

		image = TTF_RenderText_Blended(this->_font, this->_message.c_str(), { 255, 255, 255, 255 });

		if (image == nullptr)
		{
			std::cerr << "Cannot transport TTF: " << SDL_GetError() << ", failed" << std::endl;
			exit(ygl::error::ERROR_FAILED);
		}

		glGenTextures(1, &this->_data);
		glBindTexture(GL_TEXTURE_2D, this->_data);

		GLint internal_format = 0;
		GLint format = 0;

		switch (image->format->BytesPerPixel)
		{
			case 1:
				internal_format = GL_LUMINANCE;
				format = GL_RED;
				break;
			case 2:
				internal_format = GL_LUMINANCE_ALPHA;
				format = GL_RG;
				break;
			case 3:
			internal_format = GL_RGB;
#ifdef __APPLE__
				format = GL_BGR;
#else
				format = GL_RGB;
#endif
				break;
			case 4:
				internal_format = GL_RGBA;
#ifdef __APPLE__
				format = GL_BGRA;
#else
				format = GL_RGBA;
#endif
				break;
			default:
				std::cerr << "ERROR: Unknown image format" << std::endl;
				exit(ygl::error::type::ERROR_LOAD_IMAGE);
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, image->w % 4 == 0 ? 4 : 1);
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);

		// Insert it in polygon !!!!
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

		this->_size.fill((double)image->w, (double)image->h, 0.0);
		this->_map.fill(this->_size.w() / (double)image->w, this->_size.h() / (double)image->h);

		SDL_FreeSurface(image);

		double x = ygl::math::invert(this->_size.w() * this->_pivot.x());
		double y = ygl::math::invert(this->_size.h() * this->_pivot.y());
		double z = ygl::math::invert(this->_size.l() * this->_pivot.z());

		double w = x + this->_size.w();
		double h = y + this->_size.h();

		double diff = 0.0;

		switch(this->_align)
		{
			case ygl::graphic::align::ALIGN_CENTER: diff = w / 2.0; break;
			case ygl::graphic::align::ALIGN_RIGHT:  diff = w;       break;
			case ygl::graphic::align::ALIGN_LEFT:   diff = 0.0;     break;
			default:                                                 break;
		}

		this->_offset.x(0.0);
		this->_offset.y(0.0);
		this->_offset.w(this->_map.u());
		this->_offset.h(this->_map.v());

		this->_vertex.a(x - diff, y, z);
		this->_vertex.b(w - diff, y, z);
		this->_vertex.c(w - diff, h, z);
		this->_vertex.d(x - diff, h, z);
	}
}


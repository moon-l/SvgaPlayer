#define GLEW_STATIC
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "svgaglcanvas.h"
#include "svgapath.h"

#define GL_WIN_CLASS L"SvgaGLCanvasClass"
#define GL_WIN_NAME L"SvgaGLCanvas"

#define SHADER_CODE(x) #x

const char* vertexShaderSource = SHADER_CODE(
#version 330 core\n
layout (location = 0) in vec2 aPos;\n
layout (location = 1) in vec2 aTexCoords;\n

out vec2 TexCoords;\n

uniform mat4 model;\n
uniform mat4 view;\n

void main()\n
{\n
	TexCoords = aTexCoords;\n
	vec4 pos = view * model * vec4(aPos, 0.0, 1.0);\n
	gl_Position = vec4(pos.xy, 0.0, 1.0);\n
}
);

const char* fragmentShaderSource = SHADER_CODE(
#version 330 core\n

in vec2 TexCoords;\n

uniform sampler2D tex;\n
uniform float alpha;\n

void main()\n
{\n
	vec4 color = texture(tex, TexCoords);\n
	gl_FragColor = vec4(color.rgb, color.a * alpha);\n
}
);

struct GLTextureInfo
{
	QString	clipPath;
	GLuint	texture;

	GLTextureInfo() : texture(0)
	{

	}
};

class SvgaGLCanvasPrivate
{
public:
	SvgaGLCanvasPrivate(SvgaGLCanvas* q);
	~SvgaGLCanvasPrivate();

	bool init();
	void release();

	void creat(HWND parent);

	bool setup(int width, int height);

	void begin();
	void end();

	void draw(DrawItem* item);

private:
	void _initShader();
	GLuint _loadPixmap(QPixmap& pix);
	bool _replaceTexture(GLuint texture, QPixmap& pix);
	GLuint _getTexture(const QString& key, QPixmap& pix, const QString& clipPath);

private:
	SvgaGLCanvas* q_ptr;
	Q_DECLARE_PUBLIC(SvgaGLCanvas)

private:
	HWND					m_hwnd;
	HDC						m_hDC;
	HGLRC					m_hRC;

	GLuint					m_shader;
	GLuint					m_vao;
	GLuint					m_vbo;
	float					m_vertex[8];
	QMap<QString, GLTextureInfo>	m_textures;

	int						m_videoWidth;
	int						m_videoHeight;
};

SvgaGLCanvasPrivate::SvgaGLCanvasPrivate(SvgaGLCanvas* q)
: q_ptr(q)
, m_hwnd(NULL)
, m_hDC(NULL)
, m_hRC(NULL)
, m_shader(0)
, m_vao(0)
, m_vbo(0)
, m_videoWidth(0)
, m_videoHeight(0)
{

}

SvgaGLCanvasPrivate::~SvgaGLCanvasPrivate()
{
	
}

bool SvgaGLCanvasPrivate::init()
{
	return true;
}

void SvgaGLCanvasPrivate::release()
{
	for (QMap<QString, GLTextureInfo>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		GLuint id = iter.value().texture;
		glDeleteTextures(1, &id);
	}
	m_textures.clear();

	if (m_vao)
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	if (m_vbo)
	{
		glDeleteBuffers(1, &m_vbo);
		m_vbo = 0;
	}

	if (m_shader)
	{
		glDeleteProgram(m_shader);
		m_shader = 0;
	}

	wglMakeCurrent(NULL, NULL);

	if (m_hRC)
	{
		wglDeleteContext(m_hRC);
		m_hRC = NULL;
	}

	if (m_hDC)
	{
		ReleaseDC(m_hwnd, m_hDC);
		m_hDC = NULL;
	}

	if (m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

void SvgaGLCanvasPrivate::creat(HWND parent)
{
	static bool bRegister = false;
	if (!bRegister)
	{
		bRegister = true;

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= DefWindowProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= GetModuleHandle(NULL);
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= GL_WIN_CLASS;
		wcex.hIconSm		= NULL;
		RegisterClassEx(&wcex);
	}

	m_hwnd = CreateWindow(GL_WIN_CLASS, GL_WIN_NAME, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, GetModuleHandle(NULL), NULL);
	if (!m_hwnd)
	{
		return;
	}

	m_hDC = GetDC(m_hwnd);
	if (!m_hDC)
	{
		release();
		return;
	}

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
		1,                     // version number
		PFD_DRAW_TO_WINDOW |   // support window
		PFD_SUPPORT_OPENGL |   // support OpenGL
		PFD_DOUBLEBUFFER,      // double buffered
		PFD_TYPE_RGBA,         // RGBA type
		24,                    // 24-bit color depth
		0, 0, 0, 0, 0, 0,      // color bits ignored
		0,                     // no alpha buffer
		0,                     // shift bit ignored
		0,                     // no accumulation buffer
		0, 0, 0, 0,            // accum bits ignored
		32,                    // 32-bit z-buffer
		0,                     // no stencil buffer
		0,                     // no auxiliary buffer
		PFD_MAIN_PLANE,        // main layer
		0,                     // reserved
		0, 0, 0                // layer masks ignored
	};
	int nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (nPixelFormat == 0)
	{
		release();
		return;
	}

	if (!SetPixelFormat(m_hDC, nPixelFormat, &pfd))
	{
		release(); 
		return;
	}

	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
	{
		release();
		return;
	}

	wglMakeCurrent(m_hDC, m_hRC);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		release();
		return;
	}
}

bool SvgaGLCanvasPrivate::setup(int width, int height)
{
	if (width == m_videoWidth || height == m_videoHeight)
	{
		return true;
	}

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	MoveWindow(m_hwnd, rc.left, rc.top, width, height, FALSE);

	glViewport(0, 0, width, height);

	m_videoWidth = width;
	m_videoHeight = height;

	_initShader();

	float view[] = {
		2.0 / width, 0, 0, 0,
		0, -2.0 / height, 0, 0,
		0, 0, -1, 0,
		-1, 1, 0, 1
	};
	glUseProgram(m_shader);
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, view);

	glUniform1i(glGetUniformLocation(m_shader, "tex"), 0);

	float texcoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), NULL, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(8 * sizeof(float)));

	glBufferSubData(GL_ARRAY_BUFFER, sizeof(m_vertex), sizeof(texcoords), &texcoords);

	glBindVertexArray(0);

	return true;
}

void SvgaGLCanvasPrivate::begin()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void SvgaGLCanvasPrivate::end()
{
	SwapBuffers(m_hDC);
}

void SvgaGLCanvasPrivate::draw(DrawItem* item)
{
	float model[] = {
		item->transform.m11(), item->transform.m12(), 0, 0,
		item->transform.m21(), item->transform.m22(), 0, 0,
		0, 0, 1, 0,
		item->transform.dx(), item->transform.dy(), 0, 1
	};
	glUseProgram(m_shader);
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, model);

	glUniform1f(glGetUniformLocation(m_shader, "alpha"), item->alpha);

	GLuint texture = _getTexture(item->key, item->pix, item->clipPath);
	if (texture)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	m_vertex[0] = m_vertex[6] = item->layout.left();
	m_vertex[1] = m_vertex[3] = item->layout.bottom();
	m_vertex[2] = m_vertex[4] = item->layout.right();
	m_vertex[5] = m_vertex[7] = item->layout.top();

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_vertex), &m_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindVertexArray(0);
}

void SvgaGLCanvasPrivate::_initShader()
{
	int success;
	char log[512];

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, log);
	}

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, log);
	}
	
	m_shader = glCreateProgram();
	glAttachShader(m_shader, vertexShader);
	glAttachShader(m_shader, fragmentShader);
	glLinkProgram(m_shader);
	glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_shader, 512, NULL, log);
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

GLuint SvgaGLCanvasPrivate::_loadPixmap(QPixmap& pix)
{
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::ReadWrite);
	pix.save(&buffer, "png", 100);

	GLuint id;
	glGenTextures(1, &id);

	int width, height, nrComponents;
	unsigned char *data = stbi_load_from_memory((const stbi_uc*)buffer.data().data(), buffer.size(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}

	return id;
}

bool SvgaGLCanvasPrivate::_replaceTexture(GLuint texture, QPixmap& pix)
{
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::ReadWrite);
	pix.save(&buffer, "png", 100);

	int width, height, nrComponents;
	unsigned char *data = stbi_load_from_memory((const stbi_uc*)buffer.data().data(), buffer.size(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return true;
	}

	return false;
}

GLuint SvgaGLCanvasPrivate::_getTexture(const QString& key, QPixmap& pix, const QString& clipPath)
{
	SvgaPath clip;
	clip.setPath(clipPath);

	GLTextureInfo info = m_textures.value(key);
	if (!info.texture)
	{
		info.texture = _loadPixmap(clip.clip(pix));
		if (info.texture)
		{
			info.clipPath = clipPath;
			m_textures[key] = info;
		}
	}
	else if (info.clipPath != clipPath)
	{
		if (_replaceTexture(info.texture, clip.clip(pix)))
		{
			m_textures[key].clipPath = clipPath;
		}

	}

	return info.texture;
}

SvgaGLCanvas::SvgaGLCanvas()
: m_ptr(new SvgaGLCanvasPrivate(this))
{
	m_ptr->init();
}

SvgaGLCanvas::~SvgaGLCanvas()
{
	if (m_ptr)
	{
		m_ptr->release();
		delete m_ptr;
		m_ptr = NULL;
	}
}

void SvgaGLCanvas::init(HWND parent)
{
	Q_D(SvgaGLCanvas);
	d->creat(parent);
}

void SvgaGLCanvas::setPos(int x, int y)
{
	Q_D(SvgaGLCanvas);
	MoveWindow(d->m_hwnd, x, y, d->m_videoWidth, d->m_videoHeight, FALSE);
}

void SvgaGLCanvas::setVisible(bool visible)
{
	Q_D(SvgaGLCanvas);
	ShowWindow(d->m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

void SvgaGLCanvas::setVideoSize(int width, int height)
{
	Q_D(SvgaGLCanvas);
	d->setup(width, height);
}

void SvgaGLCanvas::begin()
{
	Q_D(SvgaGLCanvas);
	d->begin();
}

void SvgaGLCanvas::end()
{
	Q_D(SvgaGLCanvas);
	d->end();
}

void SvgaGLCanvas::draw(DrawItem* item)
{
	Q_D(SvgaGLCanvas);
	d->draw(item);
}

#ifdef gl_check
#define gl_start_check {
#define gl_end_check }
#else
#define gl_check(extension)
#define gl_start_check
#define gl_end_check
#endif

gl_load_operation(void, glDebugMessageCallbackARB, (GLDEBUGPROCARB callback, const void* userParam));

gl_load_operation(void, glGenBuffers, (GLsizei n, const GLuint * buffers));
gl_load_operation(void, glBufferData, (GLenum target, GLsizei ptrsize, const GLvoid * data, GLenum usage));
gl_load_operation(void, glBindBuffer, (GLenum target, GLuint buffer));
gl_load_operation(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer));
gl_load_operation(void, glEnableVertexAttribArray, (GLuint index));

gl_load_operation(void, glGenVertexArrays, (GLsizei n, GLuint *arrays));
gl_load_operation(void, glBindVertexArray, (GLuint array));

gl_load_operation(GLuint, glCreateShader, (GLenum shaderType));
gl_load_operation(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length));
gl_load_operation(void, glCompileShader, (GLuint shader));
gl_load_operation(void, glGetShaderiv, (GLuint shader, GLenum pname, GLint *params));
gl_load_operation(void, glGetShaderInfoLog, (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog));
gl_load_operation(GLuint, glCreateProgram, (void));
gl_load_operation(void, glAttachShader, (GLuint program, GLuint shader));
gl_load_operation(void, glLinkProgram, (GLuint program));
gl_load_operation(void, glGetProgramiv, (GLuint program, GLenum pname, GLint *params));
gl_load_operation(void, glGetProgramInfoLog, (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog));
gl_load_operation(void, glDetachShader, (GLuint program, GLuint shader));
gl_load_operation(void, glDeleteShader, (GLuint shader));
gl_load_operation(void, glUseProgram, (GLuint program));

gl_load_operation(void, glActiveTexture, (GLenum texture));

gl_load_operation(GLuint, glGetUniformLocation, (GLuint program, const GLchar * name));
gl_load_operation(void, glUniform1i, (GLuint location, GLint v0));
gl_load_operation(void, glUniform2f, (GLuint location, GLfloat v0, GLfloat v1));
gl_load_operation(void, glUniform3f, (GLuint location, GLfloat v0, GLfloat v1, GLfloat v2));
gl_load_operation(void, glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value));

gl_load_operation(void, wglChoosePixelFormatARB, (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats));

//TODO: fill in the arguments and outputs for these
gl_load_operation(void, glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers));
gl_load_operation(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer));
gl_load_operation(void, glRenderbufferStorage, ());
gl_load_operation(void, glGenFramebuffers, (GLsizei n, GLuint *framebuffers));
gl_load_operation(void, glBindFramebuffer, (GLenum target, GLuint framebuffer));
gl_load_operation(void, glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level));
gl_load_operation(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
gl_load_operation(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
gl_load_operation(GLenum, glCheckFramebufferStatus, (GLenum target));
gl_load_operation(void, glTexImage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations));
gl_load_operation(void, glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height));

gl_load_operation(void, glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));

gl_check("WGL_EXT_swap_control")
gl_start_check
gl_load_operation(BOOL, wglSwapIntervalEXT, (int interval));
gl_load_operation(int, wglGetSwapIntervalEXT, (void));
gl_end_check
//END TODO

gl_load_operation(HGLRC, wglCreateContextAttribsARB, (HDC hDC, HGLRC hShareContext, const int *attribList));

#undef gl_load_operation
#undef gl_check
#undef gl_start_check
#undef gl_end_check

#lang racket

(require racket/path)

(define shader_extensions '("vert" "frag"))
(define shader_path "../shaders/")
(define shader_compiler_cmdline "glslangValidator -V -o ~a ~a")
(define spv_extension ".spv")

(define (find-shaders path)
  (find-files
   (lambda (f)
     (let ([ext (filename-extension f)])
       (if ext (and (member (bytes->string/utf-8 ext) shader_extensions string=?))
           #f)))
   path)
)

(define (name->spvname n)
  (string-append (path->string n) spv_extension)
)

(for ([f (find-shaders shader_path)])
  (let ([output_name (name->spvname f)])
    (printf "Compile shader: ~a -> ~a\n" f output_name)
    (flush-output)
    (system (format shader_compiler_cmdline output_name f))))

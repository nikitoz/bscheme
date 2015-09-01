(define x (cons 10 (cons 2 (cons 3 (cons 4 (cons 5))))))

(define (sum x y) (+ x y))

(define (sum-odd-list2 xs sm)
  (if (null? xs)
	  sm
	  (if (< 0 (mod (car xs) 2))
		(sum-odd-list2 (cdr xs) sm)
	  	(sum-odd-list2 (cdr xs) (sum sm (car xs))))))

(define (sum-odd-list xs)
	(sum-odd-list2 xs 0))

(display (sum-odd-list x))
(display "hello world")
(display x)

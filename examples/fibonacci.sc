// Fibonacci en Scalin
object Main {
  def fib(n: Int): Int = match (n) {
    case 0 => 0
    case 1 => 1
    case _ => fib(n - 1) + fib(n - 2)
  }

  def main(args: Array[String]): Unit = {
    val result = fib(10)
    println(result)
  }
}
program main
  real, dimension(100) :: d
  real :: x
  integer :: i

  i = 20
  x = d(i)
  x = d(i+3) + i
end program main

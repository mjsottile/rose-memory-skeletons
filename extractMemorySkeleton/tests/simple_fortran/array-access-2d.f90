program main
  real, dimension(100,100) :: d
  real :: x
  integer :: i, j

  i = 20
  j = 0
  x = d(i,j)
  x = d(j,i) + x
  x = d(i,j) + 2*x
end program main

(module boar-param
  ()
  (import scheme (chicken base) (chicken foreign) (chicken type) srfi-4 
          typed-records)

  (foreign-declare "#include \"param.h\"")
)

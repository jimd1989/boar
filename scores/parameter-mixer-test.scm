(import boar-param boar-mixer srfi-4 srfi-18 (chicken string))

(define mix (mixer-new-from-lengths 2 2 4))
(mixer-new-master-volume-set!   mix     111.0)
(mixer-new-master-balance-set!  mix 0   222.0)
(mixer-new-master-balance-set!  mix 1   333.0)
(mixer-new-channel-volume-set!  mix 0   444.0)
(mixer-new-channel-volume-set!  mix 1   777.0)
(mixer-new-channel-balance-set! mix 0 0 555.0)
(mixer-new-channel-balance-set! mix 0 1 666.0)
(mixer-new-channel-balance-set! mix 1 0 888.0)
(mixer-new-channel-balance-set! mix 1 1 999.0)
(params-new (mixer-new-params mix))


(define par (params-from-length 3))
(params-set-linear! par 1 100.0)
(params-vector par)
(params-updated? par)
(params-after-fade-cleanup! par)



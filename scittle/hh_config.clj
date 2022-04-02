(ns hh-config
  (:require [babashka.tasks :refer [shell]]
            [hiccup.core :as hic]))

(defmacro scittle
  [& body]
  [:script {:type "application/x-scittle"}
   (hiccup.util/raw-string (cons 'do body))])

(def html
  (hic/html "<!doctype html>"
            [:html
             [:head
              [:meta {:charset "utf-8"}]
              [:meta {:http-equiv "X-UA-Compatible"
                      :content "IE=edge"}]
              [:meta {:name "viewport"
                      :content {:width "device-width"
                                :initial-scale 1.0}}]
              ;; [:meta {:http-equiv "refresh" :content "1"}]
              [:link {:rel "stylesheet"
                      :href "https://cdn.jsdelivr.net/npm/bulma@0.9/css/bulma.min.css"}]
              [:script {:src "https://cdn.jsdelivr.net/npm/scittle@0.1.2/dist/scittle.js"
                        :type "application/javascript"}]
              [:script {:crossorigin true
                        :src "https://unpkg.com/react@17/umd/react.production.min.js"}]
              [:script {:crossorigin true
                        :src "https://unpkg.com/react-dom@17/umd/react-dom.production.min.js"}]
              [:script {:src "https://cdn.jsdelivr.net/npm/scittle@0.1.2/dist/scittle.reagent.js"
                        :type "application/javascript"}]
              (scittle
               (require '[reagent.core :as r]
                        '[reagent.dom :as rdom])
               (def state (r/atom {:clicks 0}))
               (defn my-component []
                 [:div
                  [:p "Clicks: " (:clicks @state)]
                  [:button.button {:on-click #(swap! state update :clicks inc)}
                   "Click me!"]])

               (rdom/render [my-component] (.getElementById js/document "app")))
              [:title "Hacker-Hud Conf"]]
             [:body
              [:div#app]]]))

(defn spit-html
  []
  (spit "index.html" html)
  (shell "tidy -modify -indent -wrap 120 index.html"))

(defn -main
  []
  (spit-html))

(spit-html)

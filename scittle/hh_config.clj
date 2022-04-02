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
              (hiccup.util/raw-string "<script type=\"module\" src=\"https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js\"></script>")
              (hiccup.util/raw-string "<script nomodule src=\"https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js\"></script>")
              (scittle
               (require '[reagent.core :as r]
                        '[reagent.dom :as rdom])

               (def state (r/atom [0]))

               (defn my-component []
                 [:div {:style {:display "flex"
                                :flex-direction "column"}}
                  (doall
                   (for [[ix clicks] (keep-indexed vector @state)]
                     [:div {:style {:display "flex"}}
                      [:p {:style {:font-size "x-large"}}
                       "Clicks: " [:span.tag.is-large clicks]]
                      [:div {:style {:flex "1"}}]
                      [:button.button
                       {:on-click #(swap! state update ix dec)}
                       [:ion-icon {:name "remove-outline"
                                   :size "large"}]]
                      [:button.button.is-link
                       {:on-click #(swap! state update ix inc)}
                       [:ion-icon {:name "add-outline"
                                   :size "large"}]]]))
                  [:button.button.is-primary
                   {:on-click #(swap! state conj 0)}
                   [:ion-icon {:name "add-circle-outline"
                               :size "large"}]]])

               (rdom/render [my-component] (.getElementById js/document "app")))
              [:title "Hacker-Hud Conf"]]
             [:body
              [:div#app]
              ;; (hiccup.util/raw-string "<script type=\"module\" src=\"https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js\"></script>")
              ;; (hiccup.util/raw-string "<script nomodule src=\"https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js\"></script>")
              ;; [:script {:type "module"
              ;;           :src "https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js"}]
              ;; [:script (hiccup.util/raw-string "nomodule") {:src "https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js"}]
              ]]))

(defn spit-html
  []
  (spit "index.html" html)
  (try (shell "tidy -modify -indent -wrap 120 index.html")
       (catch Exception e (println e))))

(defn -main
  []
  (spit-html))

(spit-html)

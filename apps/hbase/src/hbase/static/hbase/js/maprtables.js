function referToTable(clicked){
        document.location = document.location+'/' +encodeURIComponent(clicked.id);
    }
function getCurrentCluster() {
    str = document.location.href;
    str = str.split('#')[1];
    return str;

}
$(function () {
    var tree_id = "#jstree_mapr";
    var refreshable = "needrefresh";
    $(tree_id).bind("open_node.jstree close_node.jstree", function (e,data) {
        var tree = jQuery.jstree.reference(tree_id);
        var currentNode = data.node.id;
        if(e.type === "close_node") {
            $(tree_id).jstree(true).get_node(currentNode).class = refreshable;
        }
        if(e.type === "open_node") {
            try{
                if (refreshable === $(tree_id).jstree(true).get_node(currentNode).class ) {
                    $(tree_id).jstree(true).get_node(currentNode).class = undefined;
                    $(tree_id).jstree(true).refresh_node(currentNode);
                }
            }
            catch(e){
            }
        }
    }).jstree({
        'core' : {
            'data' : {
                'url' : 'getlist/',
                'data' : function (node) {
                    return { 'id' : node.id, 'cluster' : getCurrentCluster()};
                }
            }
        }
    });
});
function referToTable(clicked){
        document.location = document.location+'/' +encodeURIComponent(clicked.id);
    }
function getCurrentCluster() {
    str = document.location.href;
    str = str.split('#')[1];
    return str;

}
$(function () {
    $('#jstree_mapr').jstree({
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
